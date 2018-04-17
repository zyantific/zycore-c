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
 * @brief   Demonstrates the `Vector` implementation.
 */

#include <stdio.h>
#include <Zycore/Allocator.h>
#include <Zycore/Defines.h>
#include <Zycore/LibC.h>
#include <Zycore/Types.h>
#include <Zycore/Vector.h>

/* ============================================================================================== */
/* Enums and types                                                                                */
/* ============================================================================================== */

/**
 * @brief   Defines the `TestStruct` struct that represents a single element in the vector.
 */
typedef struct TestStruct_
{
    ZyanU32 u32;
    ZyanU64 u64;
    float f;
} TestStruct;

/* ============================================================================================== */
/* Helper functions                                                                               */
/* ============================================================================================== */

/**
 * @brief   Initializes the given `TestStruct` struct.
 *
 * @param   data    A pointer to the `TestStruct` struct.
 * @param   n       The number to initialize the struct with.
 */
static void InitTestdata(TestStruct* data, ZyanU32 n)
{
    data->u32 = n;
    data->u64 = n;
    data->f   = (float)n;
}

/* ============================================================================================== */
/* Tests                                                                                          */
/* ============================================================================================== */

/* ---------------------------------------------------------------------------------------------- */
/* Basic tests                                                                                    */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Performs some basic test on the given `ZyanVector` instance.
 *
 * @param   vector  A pointer to the `ZyanVector` instance.
 *
 * @return  A zyan status code.
 */
static ZyanStatus PerformTest(ZyanVector* vector)
{
    static       TestStruct  e_v;
    static const TestStruct* e_p;

    // Insert `20` elements. The vector automatically manages its size
    for (ZyanU32 i = 0; i < 20; ++i)
    {
        InitTestdata(&e_v, i);
        ZYAN_CHECK(ZyanVectorPush(vector, &e_v));
    }

    // Remove elements `#05..#09`
    ZYAN_CHECK(ZyanVectorDeleteElements(vector, 5, 5));

    // Insert a new element at index `#05`
    InitTestdata(&e_v, 12345678);
    ZYAN_CHECK(ZyanVectorInsert(vector, 5, &e_v));

    // Change value of element `#15`
    InitTestdata(&e_v, 87654321);
    ZYAN_CHECK(ZyanVectorAssign(vector, 10, &e_v));

    // Print `u64` of all vector elements
    ZyanUSize size;
    ZYAN_CHECK(ZyanVectorSize(vector, &size));
    puts("ELEMENTS");
    for (ZyanUSize i = 0;  i < size; ++i)
    {
        ZYAN_CHECK(ZyanVectorGetConst(vector, i, (const void**)&e_p));
        printf("  Element #%02llu: %08llu\n", i, e_p->u64);
    }

    // Print infos
    puts("INFO");
    printf("  Size       : %08llu\n", size);
    ZYAN_CHECK(ZyanVectorCapacity(vector, &size));
    printf("  Capacity   : %08llu\n\n", size);

    return ZYAN_STATUS_SUCCESS;
}

/**
 * @brief   Performs basic tests on a vector that dynamically manages memory.
 *
 * @return  A zyan status code.
 */
static ZyanStatus TestDynamic(void)
{
    // Initialize vector with a base capacity of `10` elements
    ZyanVector vector;
    ZYAN_CHECK(ZyanVectorInit(&vector, sizeof(TestStruct), 10));

    ZYAN_CHECK(PerformTest(&vector));

    // Cleanup
    return ZyanVectorDestroy(&vector);
}

/**
 * @brief   Performs basic tests on a vector that uses a static buffer.
 *
 * @return  A zyan status code.
 */
static ZyanStatus TestStatic(void)
{
    static TestStruct buffer[20];

    // Initialize vector to use a static buffer with a total capacity of `20` elements.
    ZyanVector vector;
    ZYAN_CHECK(ZyanVectorInitBuffer(&vector, sizeof(TestStruct), buffer,
        ZYAN_ARRAY_LENGTH(buffer)));

    // Compare elements
    ZyanUSize size;
    ZYAN_CHECK(ZyanVectorSize(&vector, &size));
    for (ZyanUSize i = 0;  i < size; ++i)
    {
        static TestStruct* element;
        ZYAN_CHECK(ZyanVectorGetConst(&vector, i, (const void**)&element));
        if (element->u64 != buffer[i].u64)
        {
            return ZYAN_STATUS_INVALID_OPERATION;
        }
    }

    ZYAN_CHECK(PerformTest(&vector));

    // Cleanup
    return ZyanVectorDestroy(&vector);
}

/* ---------------------------------------------------------------------------------------------- */
/* Custom allocator                                                                               */
/* ---------------------------------------------------------------------------------------------- */

static ZyanStatus AllocatorAllocate(ZyanAllocator* allocator, void** p, ZyanUSize element_size,
    ZyanUSize n)
{
    ZYAN_ASSERT(allocator);
    ZYAN_ASSERT(p);
    ZYAN_ASSERT(element_size);
    ZYAN_ASSERT(n);

    ZYAN_UNUSED(allocator);

    *p = ZYAN_MALLOC(element_size * n);
    if (!*p)
    {
        return ZYAN_STATUS_NOT_ENOUGH_MEMORY;
    }

    return ZYAN_STATUS_SUCCESS;
}

static ZyanStatus AllocatorReallocate(ZyanAllocator* allocator, void** p, ZyanUSize element_size,
    ZyanUSize n)
{
    ZYAN_ASSERT(allocator);
    ZYAN_ASSERT(p);
    ZYAN_ASSERT(element_size);
    ZYAN_ASSERT(n);

    ZYAN_UNUSED(allocator);

    void* const x = ZYAN_REALLOC(*p, element_size * n);
    if (!x)
    {
        return ZYAN_STATUS_NOT_ENOUGH_MEMORY;
    }
    *p = x;

    return ZYAN_STATUS_SUCCESS;
}

static ZyanStatus AllocatorDeallocate(ZyanAllocator* allocator, void* p, ZyanUSize element_size,
    ZyanUSize n)
{
    ZYAN_ASSERT(allocator);
    ZYAN_ASSERT(p);
    ZYAN_ASSERT(element_size);
    ZYAN_ASSERT(n);

    ZYAN_UNUSED(allocator);
    ZYAN_UNUSED(element_size);
    ZYAN_UNUSED(n);

    ZYAN_FREE(p);

    return ZYAN_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Performs basic tests on a vector that dynamically manages memory using a custom
 *          allocator and modified growth-factor/shrink-threshold.
 *
 * @return  A zyan status code.
 */
static ZyanStatus TestAllocator(void)
{
    ZyanAllocator allocator;
    ZYAN_CHECK(ZyanAllocatorInit(&allocator, &AllocatorAllocate, &AllocatorReallocate,
        &AllocatorDeallocate));

    // Initialize vector with a base capacity of `10` elements. Growth-factor is set to 10 and
    // dynamic shrinking is disabled
    ZyanVector vector;
    ZYAN_CHECK(ZyanVectorInitEx(&vector, sizeof(TestStruct), 5, &allocator, 10.0f, 0.0f));

    static       TestStruct  e_v;
    static const TestStruct* e_p;

    // Insert `10` elements. The vector automatically manages its size
    for (ZyanU32 i = 0; i < 10; ++i)
    {
        InitTestdata(&e_v, i);
        ZYAN_CHECK(ZyanVectorPush(&vector, &e_v));
    }

    // Check capacity
    ZyanUSize size;
    ZYAN_CHECK(ZyanVectorCapacity(&vector, &size));
    if (size != 60) // (5 + 1) * 10.0f
    {
        return ZYAN_STATUS_INVALID_OPERATION;
    }

    // Remove all elements
    ZYAN_CHECK(ZyanVectorClear(&vector));

    // Print infos
    puts("INFO");
    ZYAN_CHECK(ZyanVectorSize(&vector, &size));
    printf("  Size       : %08llu\n", size);
    ZYAN_CHECK(ZyanVectorCapacity(&vector, &size));
    printf("  Capacity   : %08llu\n\n", size);

    // Cleanup
    return ZyanVectorDestroy(&vector);
}

/* ---------------------------------------------------------------------------------------------- */

/* ============================================================================================== */
/* Entry point                                                                                    */
/* ============================================================================================== */

int main()
{
    if (!ZYAN_SUCCESS(TestDynamic()))
    {
        return EXIT_FAILURE;
    }
    if (!ZYAN_SUCCESS(TestStatic()))
    {
        return EXIT_FAILURE;
    }
    if (!ZYAN_SUCCESS(TestAllocator()))
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* ============================================================================================== */
