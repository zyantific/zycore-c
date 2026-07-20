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

/**
 * @file
 * @brief   Tests the number formatting functions in `Format.h`.
 */

#include <cstdio>
#include <string>
#include <gtest/gtest.h>
#include <Zycore/Format.h>
#include <Zycore/String.h>
#include <Zycore/Types.h>

/* ============================================================================================== */
/* Helper functions                                                                               */
/* ============================================================================================== */

static const ZyanU64 POWERS_OF_TEN[20] =
{
    1ULL, 10ULL, 100ULL, 1000ULL, 10000ULL, 100000ULL, 1000000ULL, 10000000ULL, 100000000ULL,
    1000000000ULL, 10000000000ULL, 100000000000ULL, 1000000000000ULL, 10000000000000ULL,
    100000000000000ULL, 1000000000000000ULL, 10000000000000000ULL, 100000000000000000ULL,
    1000000000000000000ULL, 10000000000000000000ULL
};

static std::string FormatDecU(ZyanU64 value, ZyanU8 padding) {
    ZyanString string;
    EXPECT_EQ(ZyanStringInit(&string, 0), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(ZyanStringAppendDecU(&string, value, padding), ZYAN_STATUS_SUCCESS);
    const char* data = nullptr;
    EXPECT_EQ(ZyanStringGetData(&string, &data), ZYAN_STATUS_SUCCESS);
    const std::string result = data ? data : "";
    EXPECT_EQ(ZyanStringDestroy(&string), ZYAN_STATUS_SUCCESS);
    return result;
}

static std::string FormatHexU(ZyanU64 value, ZyanU8 padding, ZyanBool uppercase) {
    ZyanString string;
    EXPECT_EQ(ZyanStringInit(&string, 0), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(ZyanStringAppendHexU(&string, value, padding, uppercase), ZYAN_STATUS_SUCCESS);
    const char* data = nullptr;
    EXPECT_EQ(ZyanStringGetData(&string, &data), ZYAN_STATUS_SUCCESS);
    const std::string result = data ? data : "";
    EXPECT_EQ(ZyanStringDestroy(&string), ZYAN_STATUS_SUCCESS);
    return result;
}

static std::string FormatDecS(ZyanI64 value, ZyanU8 padding, ZyanBool force_sign,
    const ZyanStringView* prefix) {
    ZyanString string;
    EXPECT_EQ(ZyanStringInit(&string, 0), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(ZyanStringAppendDecS(&string, value, padding, force_sign, prefix),
        ZYAN_STATUS_SUCCESS);
    const char* data = nullptr;
    EXPECT_EQ(ZyanStringGetData(&string, &data), ZYAN_STATUS_SUCCESS);
    const std::string result = data ? data : "";
    EXPECT_EQ(ZyanStringDestroy(&string), ZYAN_STATUS_SUCCESS);
    return result;
}

static std::string FormatHexS(ZyanI64 value, ZyanU8 padding, ZyanBool uppercase,
    ZyanBool force_sign, const ZyanStringView* prefix) {
    ZyanString string;
    EXPECT_EQ(ZyanStringInit(&string, 0), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(ZyanStringAppendHexS(&string, value, padding, uppercase, force_sign, prefix),
        ZYAN_STATUS_SUCCESS);
    const char* data = nullptr;
    EXPECT_EQ(ZyanStringGetData(&string, &data), ZYAN_STATUS_SUCCESS);
    const std::string result = data ? data : "";
    EXPECT_EQ(ZyanStringDestroy(&string), ZYAN_STATUS_SUCCESS);
    return result;
}

static void CheckDecU(ZyanU64 value, ZyanU8 padding) {
    char expected[64];
    std::snprintf(expected, sizeof(expected), "%0*llu", static_cast<int>(padding),
        static_cast<unsigned long long>(value));
    EXPECT_EQ(FormatDecU(value, padding), std::string(expected))
        << "value=" << value << " padding=" << static_cast<int>(padding);
}

static void CheckHexU(ZyanU64 value, ZyanU8 padding, ZyanBool uppercase) {
    char expected[64];
    std::snprintf(expected, sizeof(expected), uppercase ? "%0*llX" : "%0*llx",
        static_cast<int>(padding), static_cast<unsigned long long>(value));
    EXPECT_EQ(FormatHexU(value, padding, uppercase), std::string(expected))
        << "value=" << value << " padding=" << static_cast<int>(padding);
}

/* ============================================================================================== */
/* Tests                                                                                          */
/* ============================================================================================== */

/* ---------------------------------------------------------------------------------------------- */
/* Decimal                                                                                        */
/* ---------------------------------------------------------------------------------------------- */

TEST(FormatTest, DecUnsignedSweep)
{
    for (ZyanU64 value = 0; value < 100000; ++value)
    {
        CheckDecU(value, 0);
    }
}

TEST(FormatTest, DecUnsignedBoundaries)
{
    for (const ZyanU64 power : POWERS_OF_TEN)
    {
        CheckDecU(power - 1, 0);
        CheckDecU(power, 0);
        CheckDecU(power + 1, 0);
    }
    CheckDecU(99999999ULL, 0);
    CheckDecU(100000000ULL, 0);
    CheckDecU(9999999999999999ULL, 0);
    CheckDecU(10000000000000000ULL, 0);
    CheckDecU(12345678901234567890ULL, 0);
    CheckDecU(18446744073709551615ULL, 0);
}

TEST(FormatTest, DecUnsignedPadding)
{
    for (const ZyanU8 padding : { 0, 1, 3, 8, 20, 25 })
    {
        CheckDecU(0, static_cast<ZyanU8>(padding));
        CheckDecU(7, static_cast<ZyanU8>(padding));
        CheckDecU(12345, static_cast<ZyanU8>(padding));
        CheckDecU(18446744073709551615ULL, static_cast<ZyanU8>(padding));
    }
}

TEST(FormatTest, DecSigned)
{
    EXPECT_EQ(FormatDecS(-12345, 0, ZYAN_FALSE, nullptr), "-12345");
    EXPECT_EQ(FormatDecS(42, 0, ZYAN_TRUE, nullptr), "+42");
    EXPECT_EQ(FormatDecS(42, 0, ZYAN_FALSE, nullptr), "42");
    EXPECT_EQ(FormatDecS(-42, 4, ZYAN_FALSE, nullptr), "-0042");

    ZyanStringView prefix;
    EXPECT_EQ(ZyanStringViewInsideBuffer(&prefix, "0n"), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(FormatDecS(-42, 0, ZYAN_FALSE, &prefix), "-0n42");
}

/* ---------------------------------------------------------------------------------------------- */
/* Hexadecimal                                                                                    */
/* ---------------------------------------------------------------------------------------------- */

TEST(FormatTest, HexUnsignedSweep)
{
    for (ZyanU64 value = 0; value < 100000; ++value)
    {
        CheckHexU(value, 0, ZYAN_FALSE);
        CheckHexU(value, 0, ZYAN_TRUE);
    }
}

TEST(FormatTest, HexUnsignedBoundaries)
{
    ZyanU64 power = 1;
    for (ZyanU8 i = 0; i < 16; ++i)
    {
        CheckHexU(power - 1, 0, ZYAN_FALSE);
        CheckHexU(power, 0, ZYAN_FALSE);
        CheckHexU(power, 0, ZYAN_TRUE);
        power <<= 4;
    }
    CheckHexU(0xFFFFFFFFULL, 0, ZYAN_FALSE);
    CheckHexU(0x100000000ULL, 0, ZYAN_FALSE);
    CheckHexU(0x0123456789ABCDEFULL, 0, ZYAN_TRUE);
    CheckHexU(18446744073709551615ULL, 0, ZYAN_FALSE);
    CheckHexU(18446744073709551615ULL, 0, ZYAN_TRUE);
}

TEST(FormatTest, HexUnsignedPadding)
{
    for (const ZyanU8 padding : { 0, 1, 4, 8, 16, 20 })
    {
        CheckHexU(0, static_cast<ZyanU8>(padding), ZYAN_FALSE);
        CheckHexU(0x1F, static_cast<ZyanU8>(padding), ZYAN_FALSE);
        CheckHexU(0x1F, static_cast<ZyanU8>(padding), ZYAN_TRUE);
        CheckHexU(0xDEADBEEFULL, static_cast<ZyanU8>(padding), ZYAN_FALSE);
        CheckHexU(18446744073709551615ULL, static_cast<ZyanU8>(padding), ZYAN_TRUE);
    }
}

TEST(FormatTest, HexSigned)
{
    EXPECT_EQ(FormatHexS(-255, 0, ZYAN_FALSE, ZYAN_FALSE, nullptr), "-ff");
    EXPECT_EQ(FormatHexS(-4096, 4, ZYAN_FALSE, ZYAN_FALSE, nullptr), "-1000");

    ZyanStringView prefix;
    EXPECT_EQ(ZyanStringViewInsideBuffer(&prefix, "0x"), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(FormatHexS(255, 0, ZYAN_TRUE, ZYAN_TRUE, &prefix), "+0xFF");
    EXPECT_EQ(FormatHexS(0, 0, ZYAN_FALSE, ZYAN_FALSE, &prefix), "0x0");
}

/* ---------------------------------------------------------------------------------------------- */

/* ============================================================================================== */
/* Entry point                                                                                    */
/* ============================================================================================== */

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

/* ============================================================================================== */
