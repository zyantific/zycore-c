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

#include <time.h>
#include <gtest/gtest.h>
#include <Zycore/Comparison.h>
#include <Zycore/Vector.h>

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

    // Custom capacity
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
/* Element access                                                                                 */
/* ---------------------------------------------------------------------------------------------- */

TEST(Vector, ElementAccess)
{
    ZyanVector vector;

    // Dynamic vector
    EXPECT_EQ(ZyanVectorInit(&vector, sizeof(ZyanU64), 0), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(vector.size, 0);

    // Write access
    const ZyanU64 element_in = 1337;
    EXPECT_EQ(ZyanVectorSetElement(&vector, 0, &element_in), ZYAN_STATUS_OUT_OF_RANGE);
    EXPECT_EQ(ZyanVectorResize(&vector, 10), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(vector.size, 10);
    EXPECT_EQ(ZyanVectorSetElement(&vector, 10, &element_in), ZYAN_STATUS_OUT_OF_RANGE);
    EXPECT_EQ(ZyanVectorSetElement(&vector, 9, &element_in), ZYAN_STATUS_SUCCESS);

    // Read access
    const ZyanU64* element_out;
    EXPECT_EQ(ZyanVectorGetElement(&vector, 10, reinterpret_cast<const void**>(&element_out)),
        ZYAN_STATUS_OUT_OF_RANGE);
    EXPECT_EQ(ZyanVectorGetElement(&vector, 9, reinterpret_cast<const void**>(&element_out)),
        ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(*element_out, element_in);

    ZyanU64* element_out_mut;
    EXPECT_EQ(ZyanVectorGetElementMutable(&vector, 10, reinterpret_cast<void**>(&element_out_mut)),
        ZYAN_STATUS_OUT_OF_RANGE);
    EXPECT_EQ(ZyanVectorGetElementMutable(&vector, 9, reinterpret_cast<void**>(&element_out_mut)),
        ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(*element_out_mut, element_in);

    *element_out_mut = 42;
    EXPECT_EQ(ZyanVectorGetElement(&vector, 9, reinterpret_cast<const void**>(&element_out)),
        ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(*element_out, *element_out_mut);

    EXPECT_EQ(ZyanVectorDestroy(&vector), ZYAN_STATUS_SUCCESS);

    // Custom buffer
    ZyanU64 buffer[10] =
    {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9
    };
    EXPECT_EQ(ZyanVectorInitCustomBuffer(&vector, sizeof(ZyanU64), &buffer, sizeof(buffer)),
        ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(vector.size, 0);

    // Write access
    EXPECT_EQ(ZyanVectorSetElement(&vector, 0, &element_in), ZYAN_STATUS_OUT_OF_RANGE);
    EXPECT_EQ(ZyanVectorResize(&vector, 10), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(vector.size, 10);
    EXPECT_EQ(ZyanVectorSetElement(&vector, 10, &element_in), ZYAN_STATUS_OUT_OF_RANGE);
    EXPECT_EQ(ZyanVectorSetElement(&vector, 9, &element_in), ZYAN_STATUS_SUCCESS);

    // Read access
    EXPECT_EQ(ZyanVectorGetElement(&vector, 10, reinterpret_cast<const void**>(&element_out)),
        ZYAN_STATUS_OUT_OF_RANGE);
    EXPECT_EQ(ZyanVectorGetElement(&vector, 9, reinterpret_cast<const void**>(&element_out)),
        ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(*element_out, element_in);
    EXPECT_EQ(buffer[9], element_in);
    // Preserved element
    EXPECT_EQ(ZyanVectorGetElement(&vector, 4, reinterpret_cast<const void**>(&element_out)),
        ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(*element_out, buffer[4]);

    EXPECT_EQ(ZyanVectorDestroy(&vector), ZYAN_STATUS_SUCCESS);
}

/* ---------------------------------------------------------------------------------------------- */
/* Insertion                                                                                      */
/* ---------------------------------------------------------------------------------------------- */

TEST(Vector, Insertion)
{
    ZyanVector vector;

    EXPECT_EQ(ZyanVectorInit(&vector, sizeof(ZyanU64), 0), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(vector.size, 0);

    for (ZyanU64 i = 0; i < 10; ++i)
    {
        EXPECT_EQ(ZyanVectorPush(&vector, &i), ZYAN_STATUS_SUCCESS);
    }
    EXPECT_EQ(vector.size, 10);

    for (ZyanUSize i = 0; i < vector.size; ++i)
    {
        const ZyanU64* element;
        EXPECT_EQ(ZyanVectorGetElement(&vector, i, reinterpret_cast<const void**>(&element)),
            ZYAN_STATUS_SUCCESS);
        EXPECT_EQ(*element, i);
    }

    static const ZyanU64 buffer[5] =
    {
        100, 101, 102, 103, 104
    };
    EXPECT_EQ(ZyanVectorInsertEx(&vector, 5, buffer, ZYAN_ARRAY_LENGTH(buffer)),
        ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(vector.size, 15);

    for (ZyanUSize i = 0; i < vector.size; ++i)
    {
        const ZyanU64* element;
        EXPECT_EQ(ZyanVectorGetElement(&vector, i, reinterpret_cast<const void**>(&element)),
            ZYAN_STATUS_SUCCESS);
        if (i < 5)
        {
            EXPECT_EQ(*element, i);
        } else
        if (i < 10)
        {
            EXPECT_EQ(*element, i + 100 - 5);
        }  else
        {
            EXPECT_EQ(*element, i - 5);
        }
    }

    const ZyanU64 element_in = 1337;
    EXPECT_EQ(ZyanVectorInsert(&vector, 0, &element_in), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(vector.size, 16);
    const ZyanU64* element_out;
    EXPECT_EQ(ZyanVectorGetElement(&vector, 0, reinterpret_cast<const void**>(&element_out)),
        ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(*element_out, element_in);

    EXPECT_EQ(ZyanVectorDestroy(&vector), ZYAN_STATUS_SUCCESS);
}

/* ---------------------------------------------------------------------------------------------- */
/* Deletion                                                                                       */
/* ---------------------------------------------------------------------------------------------- */

TEST(Vector, Deletion)
{
    ZyanVector vector;

    EXPECT_EQ(ZyanVectorInit(&vector, sizeof(ZyanU64), 0), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(vector.size, 0);

    for (ZyanU64 i = 0; i < 10; ++i)
    {
        EXPECT_EQ(ZyanVectorPush(&vector, &i), ZYAN_STATUS_SUCCESS);
    }
    EXPECT_EQ(vector.size, 10);

    // Simple deletion
    EXPECT_EQ(ZyanVectorDelete(&vector, 5), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(vector.size, 9);

    for (ZyanUSize i = 0; i < vector.size; ++i)
    {
        const ZyanU64* element;
        EXPECT_EQ(ZyanVectorGetElement(&vector, i, reinterpret_cast<const void**>(&element)),
            ZYAN_STATUS_SUCCESS);
        if (i < 5)
        {
            EXPECT_EQ(*element, i);
        } else
        {
            EXPECT_EQ(*element, i + 1);
        }
    }

    // Deletion of multiple elements
    EXPECT_EQ(ZyanVectorDeleteEx(&vector, 1, 3), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(vector.size, 6);

    static const ZyanU64 compare[6] =
    {
        0, 4, 6, 7, 8, 9
    };
    for (ZyanUSize i = 0; i < vector.size; ++i)
    {
        const ZyanU64* element;
        EXPECT_EQ(ZyanVectorGetElement(&vector, i, reinterpret_cast<const void**>(&element)),
            ZYAN_STATUS_SUCCESS);
        EXPECT_EQ(*element, compare[i]);
    }

    // Remove last
    EXPECT_EQ(ZyanVectorPop(&vector), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(vector.size, 5);

    // Clear
    EXPECT_EQ(ZyanVectorClear(&vector), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(vector.size, 0);

    EXPECT_EQ(ZyanVectorDestroy(&vector), ZYAN_STATUS_SUCCESS);
}

/* ---------------------------------------------------------------------------------------------- */
/* Searching                                                                                      */
/* ---------------------------------------------------------------------------------------------- */

TEST(Vector, Find)
{
    ZyanVector vector;

    EXPECT_EQ(ZyanVectorInit(&vector, sizeof(ZyanU64), 0), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(vector.size, 0);

    for (ZyanU64 i = 0; i < 10; ++i)
    {
        EXPECT_EQ(ZyanVectorPush(&vector, &i), ZYAN_STATUS_SUCCESS);
    }
    EXPECT_EQ(vector.size, 10);

    ZyanISize index;
    ZyanU64 element_in = 7;
    EXPECT_EQ(ZyanVectorFind(&vector, &element_in, &index,
        reinterpret_cast<ZyanEqualityComparison>(&ZyanEqualsNumeric64)), ZYAN_STATUS_TRUE);
    EXPECT_EQ(index, 7);

    element_in = 1337;
    EXPECT_EQ(ZyanVectorFind(&vector, &element_in, &index,
        reinterpret_cast<ZyanEqualityComparison>(&ZyanEqualsNumeric64)), ZYAN_STATUS_FALSE);
    EXPECT_EQ(index, -1);

    // Edge cases
    EXPECT_EQ(ZyanVectorFindEx(&vector, &element_in, &index,
        reinterpret_cast<ZyanEqualityComparison>(&ZyanEqualsNumeric64), 0, 0),
        ZYAN_STATUS_FALSE);
    EXPECT_EQ(ZyanVectorFindEx(&vector, &element_in, &index,
        reinterpret_cast<ZyanEqualityComparison>(&ZyanEqualsNumeric64), 0, 11),
        ZYAN_STATUS_OUT_OF_RANGE);
    EXPECT_EQ(ZyanVectorFindEx(&vector, &element_in, &index,
        reinterpret_cast<ZyanEqualityComparison>(&ZyanEqualsNumeric64), 1, 10),
        ZYAN_STATUS_OUT_OF_RANGE);

    EXPECT_EQ(ZyanVectorDestroy(&vector), ZYAN_STATUS_SUCCESS);
}

TEST(Vector, BinarySearch)
{
    ZyanVector vector;

    EXPECT_EQ(ZyanVectorInit(&vector, sizeof(ZyanU64), 100), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(vector.capacity, 100);
    EXPECT_EQ(vector.size, 0);

    for (ZyanUSize i = 0; i < 100; ++i)
    {
        const ZyanU64 element = rand() % 100;

        ZyanUSize index;
        const ZyanStatus status = ZyanVectorBinarySearch(&vector, &element, &index,
            reinterpret_cast<ZyanComparison>(&ZyanCompareNumeric64));
        EXPECT_EQ(ZYAN_SUCCESS(status), ZYAN_TRUE);
        EXPECT_EQ(ZyanVectorInsert(&vector, index, &element), ZYAN_STATUS_SUCCESS);
    }

    EXPECT_EQ(vector.size, 100);

    const ZyanU64* element_out;
    EXPECT_EQ(ZyanVectorGetElement(&vector, 0, reinterpret_cast<const void**>(&element_out)),
        ZYAN_STATUS_SUCCESS);
    for (ZyanUSize i = 1; i < vector.size; ++i)
    {
        const ZyanU64 value = *element_out;
        EXPECT_EQ(ZyanVectorGetElement(&vector, i, reinterpret_cast<const void**>(&element_out)),
            ZYAN_STATUS_SUCCESS);
        EXPECT_GE(*element_out, value);
    }

    // Edge cases
    const ZyanU64 element_in = 1337;
    ZyanUSize index;
    EXPECT_EQ(ZyanVectorBinarySearchEx(&vector, &element_in, &index,
        reinterpret_cast<ZyanComparison>(&ZyanCompareNumeric64), 0, 101), ZYAN_STATUS_OUT_OF_RANGE);
    EXPECT_EQ(ZyanVectorBinarySearchEx(&vector, &element_in, &index,
        reinterpret_cast<ZyanComparison>(&ZyanCompareNumeric64), 1, 100), ZYAN_STATUS_OUT_OF_RANGE);

    EXPECT_EQ(ZyanVectorDestroy(&vector), ZYAN_STATUS_SUCCESS);
}

/* ---------------------------------------------------------------------------------------------- */

/* ============================================================================================== */
/* Entry point                                                                                    */
/* ============================================================================================== */

int main(int argc, char **argv)
{
    time_t t;
    srand(static_cast<unsigned>(time(&t)));

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

/* ============================================================================================== */
