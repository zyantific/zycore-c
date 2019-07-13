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

/**
 * @brief   Definition of a single argument.
 */
typedef struct ZyanArgParseDefinition_
{
    /**
     * @brief   The argument name, e.g. `--help`.
     *
     * Must start with either one or two dashes. Single dash arguments must consist of a single
     * character, (e.g. `-n`), double-dash arguments can be of arbitrary length.
     */
    const char* name;
    /**
     * @brief   Whether the argument is boolean or expects a value.
     */
    ZyanBool boolean;
} ZyanArgParseDefinition;

typedef struct ZyanArgParseConfig_
{
    /**
     * @brief   `argv` argument passed to `main` by LibC.
     */
    const char** argv;
    /**
     * @brief   `argc` argument passed to `main` by LibC.
     */
    ZyanUSize argc;
    /**
     * @brief   Minimum # of accepted unnamed / anonymous arguments.
     */
    ZyanUSize min_unnamed_args;
    /**
     * @brief   Maximum # of accepted unnamed / anonymous arguments.
     */
    ZyanUSize max_unnamed_args;
    /**
     * @brief   Argument definition array, or `NULL`.
     *
     * Expects a pointer to an array of `ZyanArgParseDefinition` instances. The array is
     * terminated by setting the `.name` field of the last element to `NULL`. If no named
     * arguments should be parsed, you can also set this to `NULL`.
     */
    ZyanArgParseDefinition* args;
} ZyanArgParseConfig;

typedef struct ZyanArgParseArg_
{
    /**
     * @brief   Corresponding argument definition, or `NULL` for unnamed args.
     */
    const ZyanArgParseDefinition* arg;
    /**
     * @brief   Whether the argument has a value (is non-boolean).
     */
    ZyanBool has_value;
    /**
     * @brief   If `has_value == true`, then the argument value.
     */
    ZyanStringView value;
} ZyanArgParseArg;

/* ============================================================================================== */
/* Exported functions                                                                             */
/* ============================================================================================== */

/**
 * @brief  Parse arguments according to a `ZyanArgParseConfig` definition.
 *
 * @param  cfg      Argument parser config to use.
 * @param  parsed   Receives the parsed output.
 *
 * @return A `ZyanStatus` status determining whether the parsing succeeded.
 */
ZyanStatus ZyanArgParse(const ZyanArgParseConfig *cfg, ZyanVector/*<ZyanArgParseArg>*/* parsed);

/* ============================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* ZYCORE_ARGPARSE_H */
