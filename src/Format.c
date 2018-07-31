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

/* ---------------------------------------------------------------------------------------------- */
/* Lookup Tables                                                                                  */
/* ---------------------------------------------------------------------------------------------- */

static const char* decimal_lookup =
    "00010203040506070809"
    "10111213141516171819"
    "20212223242526272829"
    "30313233343536373839"
    "40414243444546474849"
    "50515253545556575859"
    "60616263646566676869"
    "70717273747576777879"
    "80818283848586878889"
    "90919293949596979899";

/* ---------------------------------------------------------------------------------------------- */

/* ============================================================================================== */
/* Internal functions                                                                             */
/* ============================================================================================== */

/* ---------------------------------------------------------------------------------------------- */
/* Formatting                                                                                     */
/* ---------------------------------------------------------------------------------------------- */



/* ---------------------------------------------------------------------------------------------- */

/* ============================================================================================== */
/* Exported functions                                                                             */
/* ============================================================================================== */

///* ---------------------------------------------------------------------------------------------- */
///* Insertion                                                                                      */
///* ---------------------------------------------------------------------------------------------- */
//
//ZyanStatus ZyanStringInsertFormat(ZyanString* destination, ZyanUSize index, const char* format, ...)
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
//
///* ---------------------------------------------------------------------------------------------- */
///* Appending                                                                                      */
///* ---------------------------------------------------------------------------------------------- */
//
//ZyanStatus ZyanStringAppendFormat(ZyanString* destination, const char* format, ...)
//{
//
//}
//
///* ---------------------------------------------------------------------------------------------- */
//
//ZyanStatus ZyanStringAppendDecU(ZyanString* string, ZyanU64 value, ZyanUSize padding_length)
//{
//
//}
//
//ZyanStatus ZyanStringAppendDecS(ZyanString* string, ZyanI64 value, ZyanUSize padding_length,
//    ZyanBool force_sign, const ZyanString* prefix)
//{
//
//}
//
//ZyanStatus ZyanStringAppendHexU(ZyanString* string, ZyanU64 value, ZyanUSize padding_length,
//    ZyanBool uppercase)
//{
//
//}
//
//ZyanStatus ZyanStringInsertHexS(ZyanString* string, ZyanI64 value, ZyanUSize padding_length,
//    ZyanBool uppercase, ZyanBool force_sign, const ZyanString* prefix)
//{
//
//}

/* ---------------------------------------------------------------------------------------------- */

/* ============================================================================================== */
