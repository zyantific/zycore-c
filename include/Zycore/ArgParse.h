/***************************************************************************************************

  Zyan Core Library (Zycore-C)

  Original Author : Joel Hoener

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
 * @brief   Implements command-line argument parsing.
 */

#ifndef ZYCORE_ARGPARSE_H
#define ZYCORE_ARGPARSE_H

#include <Zycore/Types.h>
#include <Zycore/Status.h>
#include <Zycore/Vector.h>
#include <Zycore/String.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================================== */
/* Structs and other types                                                                        */
/* ============================================================================================== */

typedef struct ZyanArgument_
{
    const char* name;
    const char* help;
    ZyanBool optional;
    ZyanBool boolean;
} ZyanArgument;

typedef struct ZyanParsedArgument_
{
    const ZyanArgument* arg;  // NULL = unnamed argument
    ZyanStringView value;
} ZyanParsedArgument;

/* ============================================================================================== */
/* Exported functions                                                                             */
/* ============================================================================================== */

ZyanStatus ZyanArgParse(
    const ZyanArgument* accepted_args,
    ZyanUSize min_unnamed_args,
    ZyanUSize max_unnamed_args,
    const char** argv,
    ZyanUSize argc,
    ZyanVector/*<ZyanParsedArgument>*/* parsed,
    ZyanString** error_string
);

ZyanStatus ZyanArgParseHelpText(
    const ZyanVector/*<ZyanArgument>*/* accepted_args,
    ZyanUSize min_unnamed_args,
    ZyanUSize max_unnamed_args,
    ZyanString* help_text
);

/* ============================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* ZYCORE_ARGPARSE_H */
