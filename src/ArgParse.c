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

#include <Zycore/ArgParse.h>
#include <Zycore/LibC.h>

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
)
{
    ZYAN_ASSERT(accepted_args);
    ZYAN_ASSERT(argv);
    ZYAN_ASSERT(parsed);

    // TODO: Consider sanity sweep of accepted args.

    ZyanStatus err;

    // Initialize output vector.
    ZYAN_CHECK(ZyanVectorInit(parsed, sizeof(ZyanParsedArgument), argc));

    ZyanBool accept_dash_args = ZYAN_TRUE;
    ZyanUSize num_unnamed_args = 0;
    for (ZyanUSize i = 0; i < argc; ++i)
    {
        const char* cur_arg = argv[i];
        ZyanUSize arg_len = ZYAN_STRLEN(argv[i]);

        // Double-dash argument?
        if (accept_dash_args && arg_len >= 2 && ZYAN_MEMCMP(cur_arg, "--", 2) == 0) {
            // GNU style end of argument parsing.
            if (arg_len == 2) {
                accept_dash_args = ZYAN_FALSE;
            }
            // Regular double-dash argument.
            else {
                // Allocate parsed argument struct.
                ZyanParsedArgument* parsed_arg;
                ZYAN_CHECK(ZyanVectorEmplace(parsed, (void**)&parsed_arg, ZYAN_NULL));
                ZYAN_MEMSET(parsed_arg, 0, sizeof(*parsed_arg));

                // Find corresponding argument definition.
                for (const ZyanArgument* arg = accepted_args; arg->name; ++arg) {
                    if (ZYAN_STRCMP(arg->name, cur_arg) == 0) {
                        parsed_arg->arg = arg;
                        break;
                    }
                }

                // Search exhaused & argument not found. RIP.
                if (!parsed_arg->arg) {
                    err = ZYAN_STATUS_INVALID_ARGUMENT; // TODO: code
                    goto failure;
                }

                // Does the argument expect a value? If yes, consume next token.
                if (!parsed_arg->arg->boolean) {
                    if (i == argc - 1) {
                        err = ZYAN_STATUS_INVALID_ARGUMENT; // TODO: code
                        goto failure;
                    }
                    ZYAN_CHECK(ZyanStringViewInsideBuffer(&parsed_arg->value, argv[++i]));
                }
            }

            // Continue parsing at next token.
            continue;
        }

        // Single-dash argument?
        // TODO: How to deal with just dashes? Current code treats it as unnamed arg.
        if (accept_dash_args && arg_len > 1 && cur_arg[0] == '-') {
            // Iterate argument token chars until there are either no more chars left
            // or we encounter a non-boolean argument, in which case we consume the
            // remaining chars as its value.
            for (const char* read_ptr = cur_arg + 1; *read_ptr; ++read_ptr) {
                // Allocate parsed argument struct.
                ZyanParsedArgument* parsed_arg;
                ZYAN_CHECK(ZyanVectorEmplace(parsed, (void**)&parsed_arg, ZYAN_NULL));
                ZYAN_MEMSET(parsed_arg, 0, sizeof(*parsed_arg));

                // Find corresponding argument definition.
                for (const ZyanArgument* arg = accepted_args; arg->name; ++arg) {
                    if (ZYAN_STRLEN(arg->name) == 2 &&
                        arg->name[0] == '-' &&
                        arg->name[1] == *read_ptr
                    ) {
                        parsed_arg->arg = arg;
                        break;
                    }
                }

                // Search exhaused, no match found?
                if (!parsed_arg->arg) {
                    err = ZYAN_STATUS_INVALID_ARGUMENT; // TODO: code
                    goto failure;
                }

                // Requires value?
                if (!parsed_arg->arg->boolean) {
                    // If there are chars left, consume them (e.g. `-n1000`).
                    if (read_ptr[1]) {
                        ZYAN_CHECK(ZyanStringViewInsideBuffer(&parsed_arg->value, read_ptr + 1));
                    }
                    // If not, consume next token (e.g. `-n 1000`).
                    else {
                        if (i == argc - 1) {
                            err = ZYAN_STATUS_INVALID_ARGUMENT; // TODO: Code
                            goto failure;
                        }
                        ZYAN_CHECK(ZyanStringViewInsideBuffer(&parsed_arg->value, argv[++i]));
                    }

                    // Either way, continue with next token.
                    continue;
                }
            }
        }

        // Still here? We're looking at an unnamed argument.
        ++num_unnamed_args;
        if (num_unnamed_args > max_unnamed_args) {
            err = ZYAN_STATUS_INVALID_ARGUMENT; // TODO: Code
            goto failure;
        }

        // Allocate parsed argument struct.
        ZyanParsedArgument* parsed_arg;
        ZYAN_CHECK(ZyanVectorEmplace(parsed, (void**)&parsed_arg, ZYAN_NULL));
        ZYAN_MEMSET(parsed_arg, 0, sizeof(*parsed_arg));
        ZYAN_CHECK(ZyanStringViewInsideBuffer(&parsed_arg->value, cur_arg));
    }

    // All tokens processed. Do we have enough unnamed arguments?
    if (num_unnamed_args < min_unnamed_args) {
        err = ZYAN_STATUS_INVALID_ARGUMENT; // TODO: Status
        goto failure;
    }

    // Yay!
    return ZYAN_STATUS_SUCCESS;

failure:
    ZYAN_CHECK(ZyanVectorDestroy(parsed, ZYAN_NULL));
    return err;
}

/* ============================================================================================== */
