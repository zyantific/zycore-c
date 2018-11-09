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
 * @brief   Tests the the arg parse implementation.
 */

#include <gtest/gtest.h>
#include <Zycore/ArgParse.h>
#include <Zycore/LibC.h>

/* ============================================================================================== */
/* Tests                                                                                          */
/* ============================================================================================== */

/* ---------------------------------------------------------------------------------------------- */
/* Unnamed args                                                                                   */
/* ---------------------------------------------------------------------------------------------- */

static auto UnnamedArgTest(ZyanU64 min, ZyanU64 max)
{
    const char* argv[] = {
        "test", "a", "xxx"
    };

    ZyanArgParseConfig cfg = {
        argv, // argv
        3,    // argc
        min,  // min_unnamed_args
        max,  // max_unnamed_args
        {}    // args
    };

    ZyanVector parsed;
    ZYAN_MEMSET(&parsed, 0, sizeof(parsed));
    auto status = ZyanArgParse(&cfg, &parsed);
    return std::make_tuple(status, parsed);
}

TEST(UnnamedArgs, TooFew)
{
    auto [status, parsed] = UnnamedArgTest(5, 5);
    ASSERT_FALSE(ZYAN_SUCCESS(status));
}

TEST(UnnamedArgs, TooMany)
{
    auto [status, parsed] = UnnamedArgTest(1, 1);
    ASSERT_FALSE(ZYAN_SUCCESS(status));
}

TEST(UnnamedArgs, PerfectFit)
{
    auto [status, parsed] = UnnamedArgTest(2, 2);
    ASSERT_TRUE(ZYAN_SUCCESS(status));

    ZyanUSize size;
    ASSERT_TRUE(ZYAN_SUCCESS(ZyanVectorGetSize(&parsed, &size)));
    ASSERT_EQ(size, 2);

    ZyanArgParseArg* arg = nullptr;
    ASSERT_TRUE(ZYAN_SUCCESS(ZyanVectorGetElement(&parsed, 0, (const void**)&arg)));
    ASSERT_STREQ((const char*)arg->value.string.vector.data /* hax! */, "a");

    ASSERT_TRUE(ZYAN_SUCCESS(ZyanVectorGetElement(&parsed, 1, (const void**)&arg)));
    ASSERT_STREQ((const char*)arg->value.string.vector.data /* hax! */, "xxx");
}

/* ---------------------------------------------------------------------------------------------- */
/* Dash args                                                                                      */
/* ---------------------------------------------------------------------------------------------- */

// TODO ...

/* ============================================================================================== */
/* Entry point                                                                                    */
/* ============================================================================================== */

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

/* ============================================================================================== */
