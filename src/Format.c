/***************************************************************************************************

  Zyan Core Library (Zycore-C)

  Original Author : Florian Bernd

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.

***************************************************************************************************/

#include <Zycore/Format.h>
#include <Zycore/LibC.h>

/* ============================================================================================== */
/* Constants                                                                                      */
/* ============================================================================================== */

/* ---------------------------------------------------------------------------------------------- */
/* Defines                                                                                        */
/* ---------------------------------------------------------------------------------------------- */

#define ZYCORE_MAXCHARS_DEC_32 10
#define ZYCORE_MAXCHARS_DEC_64 20
#define ZYCORE_MAXCHARS_HEX_32  8
#define ZYCORE_MAXCHARS_HEX_64 16

/**
 * Extra capacity reserved past the formatted digits. The SWAR fixed-width (4/8-byte) stores may
 * write up to 7 bytes past the last digit; the overrun lands only in this slack, is never counted
 * in `size`, and is overwritten by the terminating '\0'.
 */
#define ZYCORE_FMT_SLACK 8

/* ---------------------------------------------------------------------------------------------- */
/* Static strings                                                                                 */
/* ---------------------------------------------------------------------------------------------- */

static const ZyanStringView STR_ADD = ZYAN_DEFINE_STRING_VIEW("+");
static const ZyanStringView STR_SUB = ZYAN_DEFINE_STRING_VIEW("-");

/* ---------------------------------------------------------------------------------------------- */

/* ============================================================================================== */
/* Internal macros                                                                                */
/* ============================================================================================== */

/**
 * Writes a terminating '\0' character at the end of the string data.
 */
#define ZYCORE_STRING_NULLTERMINATE(string) \
      *(char*)((ZyanU8*)(string)->vector.data + (string)->vector.size - 1) = '\0';

/* ============================================================================================== */
/* Internal functions                                                                             */
/* ============================================================================================== */

/* ---------------------------------------------------------------------------------------------- */
/* SWAR integer -> ASCII cores (table-free)                                                       */
/* ---------------------------------------------------------------------------------------------- */

/*
 * Table-free integer-to-ASCII via SWAR (SIMD Within A Register): the packers convert several
 * digits at once inside one 32/64-bit word. Decimal packs two digits per 16-bit lane using a
 * fixed-point reciprocal (`* 103 >> 10` == `/ 10` for a 0..99 lane); hexadecimal spreads the
 * nibbles one-per-byte and maps them to ASCII arithmetically. Each packer puts the
 * most-significant character in byte 0; `ZyanStoreLE32`/`ZyanStoreLE64` then commit the word to
 * memory in that order on any host (a plain store on little-endian, a byte-swap first on big-
 * endian) -- the only endianness-aware step, so one code path serves both.
 *
 * These are established SWAR idioms, not a single named algorithm. The branchless hex mapping (the
 * `> 9` mask plus 0x07 / 0x27 bias) is described by W. Muła:
 *   https://web.archive.org/web/20260517061800/http://0x80.pl/notesen/2010-06-09-brancheless-hex-print.html
 * The decimal packing is the classic reciprocal-per-lane SWAR itoa (T. Mathisen's lineage); the
 * divide-by-constant reciprocals are standard bit-twiddling, see Warren, "Hacker's Delight".
 */

ZYAN_INLINE void ZyanStoreLE32(void* dst, ZyanU32 word)
{
#if ZYAN_ENDIAN == ZYAN_BIG_ENDIAN
    word = ZYAN_BYTESWAP32(word);
#endif
    ZYAN_MEMCPY(dst, &word, 4);
}

ZYAN_INLINE void ZyanStoreLE64(void* dst, ZyanU64 word)
{
#if ZYAN_ENDIAN == ZYAN_BIG_ENDIAN
    word = ZYAN_BYTESWAP64(word);
#endif
    ZYAN_MEMCPY(dst, &word, 8);
}

/**
 * Converts `x` (< 10000) to 4 packed ASCII bytes, most-significant digit in byte 0.
 */
ZYAN_INLINE ZyanU32 ZyanDec4Lanes(ZyanU32 x)
{
    const ZyanU32 a    = x / 100, b = x % 100;
    const ZyanU32 p    = a | (b << 16);                     // two 2-digit values, one per 16-bit lane
    const ZyanU32 tens = ((p * 103u) >> 10) & 0x000F000Fu;  // (v * 103) >> 10 == v / 10 for v < 100
    const ZyanU32 ones = p - tens * 10u;
    return (tens | (ones << 8)) + 0x30303030u;
}

/**
 * Converts `x` (< 10^8) to 8 packed ASCII bytes, most-significant digit in byte 0.
 */
ZYAN_INLINE ZyanU64 ZyanDec8Lanes(ZyanU32 x)
{
    return (ZyanU64)ZyanDec4Lanes(x / 10000u) | ((ZyanU64)ZyanDec4Lanes(x % 10000u) << 32);
}

/**
 * Converts the 8 nibbles of `x` to 8 packed ASCII hex bytes, least-significant nibble in byte 0.
 * `bias` is 0x27 (lowercase) or 0x07 (uppercase), added to each lane > 9 to step '9'+1 onto 'a'/'A'.
 */
ZYAN_INLINE ZyanU64 ZyanHex8Lanes(ZyanU32 x, ZyanU64 bias)
{
    ZyanU64 y = x;
    y = (y | (y << 16)) & 0x0000FFFF0000FFFFULL;
    y = (y | (y <<  8)) & 0x00FF00FF00FF00FFULL;
    y = (y | (y <<  4)) & 0x0F0F0F0F0F0F0F0FULL;             // one nibble per byte
    const ZyanU64 mask = ((y + 0x0606060606060606ULL) >> 4) & 0x0101010101010101ULL; // 1 per lane > 9
    return y + 0x3030303030303030ULL + mask * bias;
}

/**
 * Returns the number of decimal digits of `x` (which must be < 10^8), 1..8.
 */
ZYAN_INLINE ZyanU8 ZyanDecDigits(ZyanU32 x)
{
    ZyanU8 d = 1;
    if (x >= 10000u) { d += 4; x /= 10000u; }
    if (x >= 100u)   { d += 2; x /= 100u; }
    if (x >= 10u)    { d += 1; }
    return d;
}

/**
 * Writes the decimal digits of `value` MSD-first into `dst` and returns the digit count (1..20).
 *
 * `value` is split into chunks of at most 8 digits; the leading chunk is shifted to drop its
 * leading zeros. `dst` needs room for the digits plus `ZYCORE_FMT_SLACK` trailing bytes, as the
 * fixed-width stores may write past the last digit.
 */
ZYAN_INLINE ZyanU8 ZyanFormatDecSWAR(char* dst, ZyanU64 value)
{
    if (value < 10000ULL)
    {
        const ZyanU8  d = ZyanDecDigits((ZyanU32)value);
        const ZyanU32 s = ZyanDec4Lanes((ZyanU32)value) >> ((4 - d) * 8);
        ZyanStoreLE32(dst, s);
        return d;
    }
    if (value < 100000000ULL)
    {
        const ZyanU8  d = ZyanDecDigits((ZyanU32)value);
        const ZyanU64 s = ZyanDec8Lanes((ZyanU32)value) >> ((8 - d) * 8);
        ZyanStoreLE64(dst, s);
        return d;
    }
    if (value < 10000000000000000ULL)
    {
        const ZyanU32 hi = (ZyanU32)(value / 100000000ULL);
        const ZyanU32 lo = (ZyanU32)(value % 100000000ULL);
        const ZyanU8  dh = ZyanDecDigits(hi);
        const ZyanU64 sh = ZyanDec8Lanes(hi) >> ((8 - dh) * 8);
        const ZyanU64 sl = ZyanDec8Lanes(lo);
        ZyanStoreLE64(dst, sh);
        ZyanStoreLE64(dst + dh, sl);
        return (ZyanU8)(dh + 8);
    }
    {
        const ZyanU32 hi  = (ZyanU32)(value / 10000000000000000ULL);   // < 1845
        const ZyanU64 rem = value % 10000000000000000ULL;
        const ZyanU32 mid = (ZyanU32)(rem / 100000000ULL);
        const ZyanU32 lo  = (ZyanU32)(rem % 100000000ULL);
        const ZyanU8  dh  = ZyanDecDigits(hi);
        const ZyanU32 sh  = ZyanDec4Lanes(hi) >> ((4 - dh) * 8);
        const ZyanU64 sm  = ZyanDec8Lanes(mid);
        const ZyanU64 sl  = ZyanDec8Lanes(lo);
        ZyanStoreLE32(dst, sh);
        ZyanStoreLE64(dst + dh, sm);
        ZyanStoreLE64(dst + dh + 8, sl);
        return (ZyanU8)(dh + 16);
    }
}

/* ---------------------------------------------------------------------------------------------- */
/* Decimal                                                                                        */
/* ---------------------------------------------------------------------------------------------- */

#if ZYAN_ARCHITECTURE_WIDTH != 64
ZyanStatus ZyanStringAppendDecU32(ZyanString* string, ZyanU32 value, ZyanU8 padding_length)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    const ZyanUSize length_target = string->vector.size;

    // Reserve worst-case width + slack, so the digits can be written before their count is known.
    const ZyanUSize reserve = ZYAN_MAX((ZyanUSize)padding_length, ZYCORE_MAXCHARS_DEC_64)
        + ZYCORE_FMT_SLACK;
    if (string->vector.size + reserve > string->vector.capacity)
    {
        ZYAN_CHECK(ZyanStringResize(string, string->vector.size + reserve - 1));
    }

    char* buffer = (char*)string->vector.data + length_target - 1;
    const ZyanUSize length_number = ZyanFormatDecSWAR(buffer, value);
    const ZyanUSize length_total  = ZYAN_MAX(length_number, (ZyanUSize)padding_length);
    if (padding_length > length_number)
    {
        const ZyanUSize pad = length_total - length_number;
        ZYAN_MEMMOVE(buffer + pad, buffer, length_number);   // right-justify, then '0'-pad the gap
        ZYAN_MEMSET(buffer, '0', pad);
    }
    string->vector.size = length_target + length_total;
    ZYCORE_STRING_NULLTERMINATE(string);

    return ZYAN_STATUS_SUCCESS;
}
#endif

ZyanStatus ZyanStringAppendDecU64(ZyanString* string, ZyanU64 value, ZyanU8 padding_length)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    const ZyanUSize length_target = string->vector.size;

    // Reserve worst-case width + slack, so the digits can be written before their count is known.
    const ZyanUSize reserve = ZYAN_MAX((ZyanUSize)padding_length, ZYCORE_MAXCHARS_DEC_64)
        + ZYCORE_FMT_SLACK;
    if (string->vector.size + reserve > string->vector.capacity)
    {
        ZYAN_CHECK(ZyanStringResize(string, string->vector.size + reserve - 1));
    }

    char* buffer = (char*)string->vector.data + length_target - 1;
    const ZyanUSize length_number = ZyanFormatDecSWAR(buffer, value);
    const ZyanUSize length_total  = ZYAN_MAX(length_number, (ZyanUSize)padding_length);
    if (padding_length > length_number)
    {
        const ZyanUSize pad = length_total - length_number;
        ZYAN_MEMMOVE(buffer + pad, buffer, length_number);   // right-justify, then '0'-pad the gap
        ZYAN_MEMSET(buffer, '0', pad);
    }
    string->vector.size = length_target + length_total;
    ZYCORE_STRING_NULLTERMINATE(string);

    return ZYAN_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------------------------- */
/* Hexadecimal                                                                                    */
/* ---------------------------------------------------------------------------------------------- */

/**
 * Returns the number of hexadecimal digits required to represent `value`.
 *
 * @param   value   The value. Must not be `0`.
 *
 * @return  The number of hexadecimal digits (1..16).
 */
ZYAN_INLINE ZyanU8 ZyanHexDigitCount(ZyanU64 value)
{
    ZYAN_ASSERT(value);
#if defined(ZYAN_GCC) || defined(ZYAN_CLANG) || defined(ZYAN_ICC)
    return (ZyanU8)(((63 - __builtin_clzll(value)) >> 2) + 1);
#else
    ZyanU8 count = 1;
    while (value >>= 4)
    {
        ++count;
    }
    return count;
#endif
}

/**
 * Writes the hexadecimal digits of `value` MSD-first into `dst` and returns the digit count (1..16).
 *
 * Each 32-bit half is packed by `ZyanHex8Lanes` (least-significant nibble in byte 0), byte-swapped
 * so the most-significant nibble lands in byte 0, then shifted to drop the leading-zero nibbles.
 * `dst` needs `ZYCORE_FMT_SLACK` trailing bytes of room (see `ZyanFormatDecSWAR`).
 */
ZYAN_INLINE ZyanU8 ZyanFormatHexSWAR(char* dst, ZyanU64 value, ZyanBool uppercase)
{
    const ZyanU8  digits = value ? ZyanHexDigitCount(value) : 1;
    const ZyanU64 bias   = uppercase ? 0x07ULL : 0x27ULL;   // '9'+1 -> 'A' / 'a' in each lane > 9
    if (value <= 0xFFFFFFFFULL)
    {
        const ZyanU64 b = ZYAN_BYTESWAP64(ZyanHex8Lanes((ZyanU32)value, bias));
        const ZyanU64 s = b >> ((8 - digits) * 8);
        ZyanStoreLE64(dst, s);
        return digits;
    }
    {
        const ZyanU8  dh = (ZyanU8)(digits - 8);           // significant nibbles in the high dword
        const ZyanU64 hi = ZYAN_BYTESWAP64(ZyanHex8Lanes((ZyanU32)(value >> 32), bias));
        const ZyanU64 sh = hi >> ((8 - dh) * 8);
        const ZyanU64 lo = ZYAN_BYTESWAP64(ZyanHex8Lanes((ZyanU32)value, bias));
        ZyanStoreLE64(dst, sh);
        ZyanStoreLE64(dst + dh, lo);
        return digits;
    }
}

#if ZYAN_ARCHITECTURE_WIDTH != 64
ZyanStatus ZyanStringAppendHexU32(ZyanString* string, ZyanU32 value, ZyanU8 padding_length,
    ZyanBool uppercase)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    const ZyanUSize length_target = string->vector.size;

    // Reserve worst-case width + slack, so the digits can be written before their count is known.
    const ZyanUSize reserve = ZYAN_MAX((ZyanUSize)padding_length, ZYCORE_MAXCHARS_HEX_64)
        + ZYCORE_FMT_SLACK;
    if (string->vector.size + reserve > string->vector.capacity)
    {
        ZYAN_CHECK(ZyanStringResize(string, string->vector.size + reserve - 1));
    }

    char* buffer = (char*)string->vector.data + length_target - 1;
    const ZyanUSize length_number = ZyanFormatHexSWAR(buffer, value, uppercase);
    const ZyanUSize length_total  = ZYAN_MAX(length_number, (ZyanUSize)padding_length);
    if (padding_length > length_number)
    {
        const ZyanUSize pad = length_total - length_number;
        ZYAN_MEMMOVE(buffer + pad, buffer, length_number);   // right-justify, then '0'-pad the gap
        ZYAN_MEMSET(buffer, '0', pad);
    }
    string->vector.size = length_target + length_total;
    ZYCORE_STRING_NULLTERMINATE(string);

    return ZYAN_STATUS_SUCCESS;
}
#endif

ZyanStatus ZyanStringAppendHexU64(ZyanString* string, ZyanU64 value, ZyanU8 padding_length,
    ZyanBool uppercase)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    const ZyanUSize length_target = string->vector.size;

    // Reserve worst-case width + slack, so the digits can be written before their count is known.
    const ZyanUSize reserve = ZYAN_MAX((ZyanUSize)padding_length, ZYCORE_MAXCHARS_HEX_64)
        + ZYCORE_FMT_SLACK;
    if (string->vector.size + reserve > string->vector.capacity)
    {
        ZYAN_CHECK(ZyanStringResize(string, string->vector.size + reserve - 1));
    }

    char* buffer = (char*)string->vector.data + length_target - 1;
    const ZyanUSize length_number = ZyanFormatHexSWAR(buffer, value, uppercase);
    const ZyanUSize length_total  = ZYAN_MAX(length_number, (ZyanUSize)padding_length);
    if (padding_length > length_number)
    {
        const ZyanUSize pad = length_total - length_number;
        ZYAN_MEMMOVE(buffer + pad, buffer, length_number);   // right-justify, then '0'-pad the gap
        ZYAN_MEMSET(buffer, '0', pad);
    }
    string->vector.size = length_target + length_total;
    ZYCORE_STRING_NULLTERMINATE(string);

    return ZYAN_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------------------------- */

/* ============================================================================================== */
/* Exported functions                                                                             */
/* ============================================================================================== */

/* ---------------------------------------------------------------------------------------------- */
/* Insertion                                                                                      */
/* ---------------------------------------------------------------------------------------------- */

//ZyanStatus ZyanStringInsertFormat(ZyanString* string, ZyanUSize index, const char* format, ...)
//{
//
//}
//
///* ---------------------------------------------------------------------------------------------- */
//
//ZyanStatus ZyanStringInsertDecU(ZyanString* string, ZyanUSize index, ZyanU64 value,
//    ZyanUSize padding_length)
//{
//
//}
//
//ZyanStatus ZyanStringInsertDecS(ZyanString* string, ZyanUSize index, ZyanI64 value,
//    ZyanUSize padding_length, ZyanBool force_sign, const ZyanString* prefix)
//{
//
//}
//
//ZyanStatus ZyanStringInsertHexU(ZyanString* string, ZyanUSize index, ZyanU64 value,
//    ZyanUSize padding_length, ZyanBool uppercase)
//{
//
//}
//
//ZyanStatus ZyanStringInsertHexS(ZyanString* string, ZyanUSize index, ZyanI64 value,
//    ZyanUSize padding_length, ZyanBool uppercase, ZyanBool force_sign, const ZyanString* prefix)
//{
//
//}

/* ---------------------------------------------------------------------------------------------- */
/* Appending                                                                                      */
/* ---------------------------------------------------------------------------------------------- */

#ifndef ZYAN_NO_LIBC

ZyanStatus ZyanStringAppendFormat(ZyanString* string, const char* format, ...)
{
    if (!string || !format)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    ZyanVAList arglist;
    ZYAN_VA_START(arglist, format);

    const ZyanUSize len = string->vector.size;

    ZyanI32 w = ZYAN_VSNPRINTF((char*)string->vector.data + len - 1,
        string->vector.capacity - len + 1, format, arglist);
    if (w < 0)
    {
        ZYAN_VA_END(arglist);
        return ZYAN_STATUS_FAILED;
    }
    if (w <= (ZyanI32)(string->vector.capacity - len))
    {
        string->vector.size = len + w;

        ZYAN_VA_END(arglist);
        return ZYAN_STATUS_SUCCESS;
    }

    // The remaining capacity was not sufficent to fit the formatted string. Trying to resize ..
    const ZyanStatus status = ZyanStringResize(string, string->vector.size + w - 1);
    if (!ZYAN_SUCCESS(status))
    {
        ZYAN_VA_END(arglist);
        return status;
    }

    w = ZYAN_VSNPRINTF((char*)string->vector.data + len - 1,
        string->vector.capacity - string->vector.size + 1, format, arglist);
    if (w < 0)
    {
        ZYAN_VA_END(arglist);
        return ZYAN_STATUS_FAILED;
    }
    ZYAN_ASSERT(w <= (ZyanI32)(string->vector.capacity - string->vector.size));

    ZYAN_VA_END(arglist);
    return ZYAN_STATUS_SUCCESS;
}

#endif // ZYAN_NO_LIBC

/* ---------------------------------------------------------------------------------------------- */

ZyanStatus ZyanStringAppendDecU(ZyanString* string, ZyanU64 value, ZyanU8 padding_length)
{
#if ZYAN_ARCHITECTURE_WIDTH == 64
    return ZyanStringAppendDecU64(string, value, padding_length);
#else
    // Working with 64-bit values is slow on non 64-bit systems
    if (value & 0xFFFFFFFF00000000)
    {
        return ZyanStringAppendDecU64(string, value, padding_length);
    }
    return ZyanStringAppendDecU32(string, (ZyanU32)value, padding_length);
#endif
}

ZyanStatus ZyanStringAppendDecS(ZyanString* string, ZyanI64 value, ZyanU8 padding_length,
    ZyanBool force_sign, const ZyanStringView* prefix)
{
    if (value < 0)
    {
        ZYAN_CHECK(ZyanStringAppend(string, &STR_SUB));
        if (prefix)
        {
            ZYAN_CHECK(ZyanStringAppend(string, prefix));
        }
        return ZyanStringAppendDecU(string, ZyanAbsI64(value), padding_length);
    }

    if (force_sign)
    {
        ZYAN_ASSERT(value >= 0);
        ZYAN_CHECK(ZyanStringAppend(string, &STR_ADD));
    }

    if (prefix)
    {
        ZYAN_CHECK(ZyanStringAppend(string, prefix));
    }
    return ZyanStringAppendDecU(string, value, padding_length);
}

ZyanStatus ZyanStringAppendHexU(ZyanString* string, ZyanU64 value, ZyanU8 padding_length,
    ZyanBool uppercase)
{
#if ZYAN_ARCHITECTURE_WIDTH == 64
    return ZyanStringAppendHexU64(string, value, padding_length, uppercase);
#else
    // Working with 64-bit values is slow on non 64-bit systems
    if (value & 0xFFFFFFFF00000000)
    {
        return ZyanStringAppendHexU64(string, value, padding_length, uppercase);
    }
    return ZyanStringAppendHexU32(string, (ZyanU32)value, padding_length, uppercase);
#endif
}

ZyanStatus ZyanStringAppendHexS(ZyanString* string, ZyanI64 value, ZyanU8 padding_length,
    ZyanBool uppercase, ZyanBool force_sign, const ZyanStringView* prefix)
{
    if (value < 0)
    {
        ZYAN_CHECK(ZyanStringAppend(string, &STR_SUB));
        if (prefix)
        {
            ZYAN_CHECK(ZyanStringAppend(string, prefix));
        }
        return ZyanStringAppendHexU(string, ZyanAbsI64(value), padding_length, uppercase);
    }

    if (force_sign)
    {
        ZYAN_ASSERT(value >= 0);
        ZYAN_CHECK(ZyanStringAppend(string, &STR_ADD));
    }

    if (prefix)
    {
        ZYAN_CHECK(ZyanStringAppend(string, prefix));
    }
    return ZyanStringAppendHexU(string, value, padding_length, uppercase);
}

/* ---------------------------------------------------------------------------------------------- */

/* ============================================================================================== */
