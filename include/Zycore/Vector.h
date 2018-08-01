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
 * @brief   Implements the vector container class.
 */

#ifndef ZYCORE_VECTOR_H
#define ZYCORE_VECTOR_H

#include <ZycoreExportConfig.h>
#include <Zycore/Allocator.h>
#include <Zycore/Comparison.h>
#include <Zycore/Status.h>
#include <Zycore/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================================== */
/* Enums and types                                                                                */
/* ============================================================================================== */

/**
 * @brief   Defines the `ZyanVector` struct.
 *
 * All fields in this struct should be considered as "private". Any changes may lead to unexpected
 * behavior.
 */
typedef struct ZyanVector_
{
    /**
     * @brief   The memory allocator.
     */
    ZyanAllocator* allocator;
    /**
     * @brief   The growth factor.
     */
    float growth_factor;
    /**
     * @brief   The shrink threshold.
     */
    float shrink_threshold;
    /**
     * @brief   The current number of elements in the vector.
     */
    ZyanUSize size;
    /**
     * @brief   The maximum capacity (number of elements).
     */
    ZyanUSize capacity;
    /**
     * @brief   The size of a single element in bytes.
     */
    ZyanUSize element_size;
    /**
     * @brief   The data pointer.
     */
    void* data;
} ZyanVector;

/* ============================================================================================== */
/* Exported functions                                                                             */
/* ============================================================================================== */

/* ---------------------------------------------------------------------------------------------- */
/* Constructor and destructor                                                                     */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Initializes the given `ZyanVector` instance.
 *
 * @param   vector          A pointer to the `ZyanVector` instance.
 * @param   element_size    The size of a single element in bytes.
 * @param   capacity        The initial capacity (number of elements).
 *
 * @return  A zyan status code.
 *
 * The memory for the vector elements is dynamically allocated by the default allocator using the
 * default growth factor of `2.0f` and the default shrink threshold of `0.25f`.
 *
 * Finalization with `ZyanVectorDestroy` is required for all instances created by this function.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorInit(ZyanVector* vector, ZyanUSize element_size,
    ZyanUSize capacity);

/**
 * @brief   Initializes the given `ZyanVector` instance and sets a custom `allocator` and memory
 *          allocation/deallocation parameters.
 *
 * @param   vector              A pointer to the `ZyanVector` instance.
 * @param   element_size        The size of a single element in bytes.
 * @param   capacity            The initial capacity (number of elements).
 * @param   allocator           A pointer to a `ZyanAllocator` instance.
 * @param   growth_factor       The growth factor (from `1.0f` to `x.xf`).
 * @param   shrink_threshold    The shrink threshold (from `0.0f` to `1.0f`).
 *
 * @return  A zyan status code.
 *
 * A growth factor of `1.0f` disables overallocation and a shrink threshold of `0.0f` disables
 * dynamic shrinking.
 *
 * Finalization with `ZyanVectorDestroy` is required for all instances created by this function.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorInitEx(ZyanVector* vector, ZyanUSize element_size,
    ZyanUSize capacity, ZyanAllocator* allocator, float growth_factor, float shrink_threshold);

/**
 * @brief   Initializes the given `ZyanVector` instance and configures it to use a custom user
 *          defined buffer with a fixed size.
 *
 * @param   vector          A pointer to the `ZyanVector` instance.
 * @param   element_size    The size of a single element in bytes.
 * @param   buffer          A pointer to the buffer that is used as storage for the elements.
 * @param   capacity        The maximum capacity (number of elements) of the buffer.
 *
 * @return  A zyan status code.
 *
 * Finalization is not required for instances created by this function.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorInitCustomBuffer(ZyanVector* vector, ZyanUSize element_size,
    void* buffer, ZyanUSize capacity);

/**
 * @brief   Destroys the given `ZyanVector` instance.
 *
 * @param   vector  A pointer to the `ZyanVector` instance.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorDestroy(ZyanVector* vector);

/* ---------------------------------------------------------------------------------------------- */
/* Element access                                                                                 */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Returns a pointer to the element at the given `index`.
 *
 * @param   vector  A pointer to the `ZyanVector` instance.
 * @param   index   The element index.
 * @param   value Receives a pointer to the desired element in the vector.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorGetElementMutable(const ZyanVector* vector, ZyanUSize index,
    void** value);

/**
 * @brief   Returns a constant pointer to the element at the given `index`.
 *
 * @param   vector  A pointer to the `ZyanVector` instance.
 * @param   index   The element index.
 * @param   value   Receives a constant pointer to the desired element in the vector.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorGetElement(const ZyanVector* vector, ZyanUSize index,
    const void** value);

/**
 * @brief   Assigns a new value to the element at the given `index`.
 *
 * @param   vector  A pointer to the `ZyanVector` instance.
 * @param   index   The value index.
 * @param   value   The value to assign.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorSetElement(ZyanVector* vector, ZyanUSize index,
    const void* value);

/* ---------------------------------------------------------------------------------------------- */
/* Insertion                                                                                      */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Adds a new `element` at the end of the vector.
 *
 * @param   vector  A pointer to the `ZyanVector` instance.
 * @param   element A pointer to the element to add.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorPush(ZyanVector* vector, const void* element);

/**
 * @brief   Inserts an `element` at the given `index` of the vector.
 *
 * @param   vector  A pointer to the `ZyanVector` instance.
 * @param   index   The insert index.
 * @param   element A pointer to the element to insert.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorInsert(ZyanVector* vector, ZyanUSize index, const void* element);

/**
 * @brief   Inserts multiple `elements` at the given `index` of the vector.
 *
 * @param   vector      A pointer to the `ZyanVector` instance.
 * @param   index       The insert index.
 * @param   elements    A pointer to the first element.
 * @param   count       The number of elements to insert.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorInsertEx(ZyanVector* vector, ZyanUSize index,
    const void* elements, ZyanUSize count);

/* ---------------------------------------------------------------------------------------------- */
/* Deletion                                                                                       */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Deletes the element at the given `index` of the vector.
 *
 * @param   vector  A pointer to the `ZyanVector` instance.
 * @param   index   The element index.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorDelete(ZyanVector* vector, ZyanUSize index);

/**
 * @brief   Deletes multiple elements from the given vector, starting at `index`.
 *
 * @param   vector  A pointer to the `ZyanVector` instance.
 * @param   index   The index of the first element to delete.
 * @param   count   The number of elements to delete.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorDeleteEx(ZyanVector* vector, ZyanUSize index, ZyanUSize count);

/**
 * @brief   Removes the last element of the vector.
 *
 * @param   vector  A pointer to the `ZyanVector` instance.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorPop(ZyanVector* vector);

/**
 * @brief   Erases all elements of the given vector.
 *
 * @param   vector  A pointer to the `ZyanVector` instance.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorClear(ZyanVector* vector);

/* ---------------------------------------------------------------------------------------------- */
/* Searching                                                                                      */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Searches for the first occurrence of `element` in the given vector.
 *
 * @param   vector      A pointer to the `ZyanVector` instance.
 * @param   element     A pointer to the element to search for.
 * @param   found_index A pointer to a variable that receives the index of the found element.
 * @param   comparison  The comparison function to use.
 *
 * @return  `ZYAN_STATUS_TRUE`, if the element was found, `ZYAN_STATUS_FALSE`, if not, or another
 *          zyan status code, if an error occured.
 *
 * The `found_index` is set to `-1`, if the element was not found.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorFind(const ZyanVector* vector, const void* element,
    ZyanISize* found_index, ZyanEqualityComparison comparison);

/**
 * @brief   Searches for the first occurrence of `element` in the given vector.
 *
 * @param   vector      A pointer to the `ZyanVector` instance.
 * @param   element     A pointer to the element to search for.
 * @param   found_index A pointer to a variable that receives the index of the found element.
 * @param   comparison  The comparison function to use.
 * @param   index       The start index.
 * @param   count       The maximum number of elements to iterate, beginning from the start `index`.
 *
 * @return  `ZYAN_STATUS_TRUE`, if the element was found, `ZYAN_STATUS_FALSE`, if not, or another
 *          zyan status code, if an error occured.
 *
 * The `found_index` is set to `-1`, if the element was not found.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorFindEx(const ZyanVector* vector, const void* element,
    ZyanISize* found_index, ZyanEqualityComparison comparison, ZyanUSize index, ZyanUSize count);

/**
 * @brief   Searches for the first occurrence of `element` in the given vector using a binary-
 *          search algorithm.
 *
 * @param   vector      A pointer to the `ZyanVector` instance.
 * @param   element     A pointer to the element to search for.
 * @param   found_index A pointer to a variable that receives the index of the found element.
 * @param   comparison  The comparison function to use.
 *
 * @return  `ZYAN_STATUS_TRUE`, if the element was found, `ZYAN_STATUS_FALSE`, if not, or another
 *          zyan status code, if an error occured.
 *
 * If found, `found_index` contains the zero-based index of `element`. If not found, `found_index`
 * contains the index of the first entry larger than `element`.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorBinarySearch(const ZyanVector* vector, const void* element,
    ZyanUSize* found_index, ZyanComparison comparison);

/**
 * @brief   Searches for the first occurrence of `element` in the given vector using a binary-
 *          search algorithm.
 *
 * @param   vector      A pointer to the `ZyanVector` instance.
 * @param   element     A pointer to the element to search for.
 * @param   found_index A pointer to a variable that receives the index of the found element.
 * @param   comparison  The comparison function to use.
 * @param   index       The start index.
 * @param   count       The maximum number of elements to iterate, beginning from the start `index`.
 *
 * @return  `ZYAN_STATUS_TRUE`, if the element was found, `ZYAN_STATUS_FALSE`, if not, or another
 *          zyan status code, if an error occured.
 *
 * If found, `found_index` contains the zero-based index of `element`. If not found, `found_index`
 * contains the index of the first entry larger than `element`.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorBinarySearchEx(const ZyanVector* vector, const void* element,
    ZyanUSize* found_index, ZyanComparison comparison, ZyanUSize index, ZyanUSize count);

/* ---------------------------------------------------------------------------------------------- */
/* Memory management                                                                              */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Resizes the given `ZyanVector` instance.
 *
 * @param   vector  A pointer to the `ZyanVector` instance.
 * @param   size    The new size of the vector.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorResize(ZyanVector* vector, ZyanUSize size);

/**
 * @brief   Changes the capacity of the given `ZyanVector` instance.
 *
 * @param   vector      A pointer to the `ZyanVector` instance.
 * @param   capacity    The new minimum capacity of the vector.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorReserve(ZyanVector* vector, ZyanUSize capacity);

/**
 * @brief   Shrinks the capacity of the given vector to match it's size.
 *
 * @param   vector  A pointer to the `ZyanVector` instance.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorShrinkToFit(ZyanVector* vector);

/* ---------------------------------------------------------------------------------------------- */
/* Information                                                                                    */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Returns the current size of the vector.
 *
 * @param   vector  A pointer to the `ZyanVector` instance.
 * @param   size    Receives the size of the vector.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorGetSize(const ZyanVector* vector, ZyanUSize* size);

/**
 * @brief   Returns the current capacity of the vector.
 *
 * @param   vector      A pointer to the `ZyanVector` instance.
 * @param   capacity    Receives the size of the vector.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanVectorGetCapacity(const ZyanVector* vector, ZyanUSize* capacity);

/* ---------------------------------------------------------------------------------------------- */

/* ============================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* ZYCORE_VECTOR_H */
