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
 * @brief   Tests the `ZyanVector` implementation.
 */

#include <gtest/gtest.h>
#include <Zycore/Vector.h>

/* ============================================================================================== */
/* Enums and types                                                                                */
/* ============================================================================================== */



/* ============================================================================================== */
/* Helper functions                                                                               */
/* ============================================================================================== */



/* ============================================================================================== */
/* Tests                                                                                          */
/* ============================================================================================== */

/* ---------------------------------------------------------------------------------------------- */
/* Constructor and destructor                                                                     */
/* ---------------------------------------------------------------------------------------------- */

TEST(Vector, InitBasic)
{
    ZyanVector vector;

    EXPECT_EQ(ZyanVectorInit(&vector, 4, 0), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(vector.allocator, ZyanAllocatorDefault());
    EXPECT_FLOAT_EQ(vector.growth_factor, 2.0f);
    EXPECT_FLOAT_EQ(vector.shrink_threshold, 0.25f);
    EXPECT_EQ(vector.size, 0);
    EXPECT_EQ(vector.capacity, 1);
    EXPECT_EQ(vector.element_size, 4);
    EXPECT_NE(vector.data, ZYAN_NULL);
    EXPECT_EQ(ZyanVectorDestroy(&vector), ZYAN_STATUS_SUCCESS);

    // Custom capacity
    EXPECT_EQ(ZyanVectorInit(&vector, 4, 10), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(vector.capacity, 10);
    EXPECT_EQ(ZyanVectorDestroy(&vector), ZYAN_STATUS_SUCCESS);
}

TEST(Vector, InitAdvanced)
{
     ZyanVector vector;

    EXPECT_EQ(ZyanVectorInitEx(&vector, 4, 0, ZyanAllocatorDefault(), 1.0f, 0.0f),
        ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(vector.allocator, ZyanAllocatorDefault());
    EXPECT_FLOAT_EQ(vector.growth_factor, 1.0f);
    EXPECT_FLOAT_EQ(vector.shrink_threshold, 0.0f);
    EXPECT_EQ(vector.size, 0);
    EXPECT_EQ(vector.capacity, 1);
    EXPECT_EQ(vector.element_size, 4);
    EXPECT_NE(vector.data, ZYAN_NULL);
    EXPECT_EQ(ZyanVectorDestroy(&vector), ZYAN_STATUS_SUCCESS);

    EXPECT_EQ(ZyanVectorInitEx(&vector, 4, 10, ZyanAllocatorDefault(), 1.0f, 0.0f),
        ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(vector.capacity, 10);
    EXPECT_EQ(ZyanVectorDestroy(&vector), ZYAN_STATUS_SUCCESS);
}

TEST(Vector, InitCustomBuffer)
{
    ZyanVector vector;

    ZyanU8 buffer[64];
    EXPECT_EQ(ZyanVectorInitCustomBuffer(&vector, 4, &buffer, sizeof(buffer)), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(vector.allocator, ZYAN_NULL);
    EXPECT_FLOAT_EQ(vector.growth_factor, 1.0f);
    EXPECT_FLOAT_EQ(vector.shrink_threshold, 0.0f);
    EXPECT_EQ(vector.size, 0);
    EXPECT_EQ(vector.capacity, sizeof(buffer));
    EXPECT_EQ(vector.element_size, 4);
    EXPECT_EQ(vector.data, &buffer);
    EXPECT_EQ(ZyanVectorDestroy(&vector), ZYAN_STATUS_SUCCESS);
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
