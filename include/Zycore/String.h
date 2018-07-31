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
 * @brief   Implements the string type.
 *
 * The `ZyanString` type is implemented as a size-prefixed string - which allows for a lot of
 * performance optimizations in different situations.
 * Nevertheless null-termination is guaranteed at all times to provide maximum compatibility with
 * default C-style strings (use `ZyanStringUnwrap` to access the C-style string).
 */

#ifndef ZYCORE_STRING_H
#define ZYCORE_STRING_H

#include <ZycoreExportConfig.h>
#include <Zycore/Allocator.h>
#include <Zycore/Status.h>
#include <Zycore/Types.h>
#include <Zycore/Vector.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================================== */
/* Enums and types                                                                                */
/* ============================================================================================== */

/* ---------------------------------------------------------------------------------------------- */
/* String flags                                                                                   */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Defines the `ZyanStringFlags` datatype.
 */
typedef ZyanU8 ZyanStringFlags;

/**
 * @brief   The string is immutable and can't be modified.
 *
 *          This flag is set by default for all wrapped C-style strings (created by
 *          `ZYAN_STRING_WRAP`, `ZyanStringWrap`).
 */
#define ZYAN_STRING_IS_IMMUTABLE    0x01 // (1 << 0)

/* ---------------------------------------------------------------------------------------------- */
/* String                                                                                         */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Defines the `ZyanString` struct.
 *
 * All fields in this struct should be considered as "private". Any changes may lead to unexpected
 * behavior.
 */
typedef struct ZyanString_
{
    /**
     * @brief   String flags.
     */
    ZyanStringFlags flags;
    /**
     * @brief   The string that contains the actual string.
     */
    ZyanVector data;
} ZyanString;

/* ---------------------------------------------------------------------------------------------- */

/* ============================================================================================== */
/* Macros                                                                                         */
/* ============================================================================================== */

/* ---------------------------------------------------------------------------------------------- */
/* Helper Macros                                                                                  */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Declares a `ZyanString` struct by wrapping a static C-style string.
 *
 * @param   string  The C-string.
 *
 * Strings declared with this macro are IMMUTABLE and do not need finalization.
 */
#define ZYAN_STRING_WRAP(string) \
    { \
        /* flags */ ZYAN_STRING_IS_IMMUTABLE, \
        /* data  */ \
        { \
            /* allocator        */ ZYAN_NULL, \
            /* growth_factor    */ 1.0f, \
            /* shrink_threshold */ 0.0f, \
            /* size             */ sizeof(string) - 1, \
            /* capacity         */ sizeof(string) - 1, \
            /* element_size     */ sizeof(char), \
            /* data             */ (char*)(string) \
        } \
    }

/* ---------------------------------------------------------------------------------------------- */

/* ============================================================================================== */
/* Exported functions                                                                             */
/* ============================================================================================== */

/* ---------------------------------------------------------------------------------------------- */
/* Constructor and destructor                                                                     */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Initializes the given `ZyanString` instance.
 *
 * @param   string          A pointer to the `ZyanString` instance.
 * @param   capacity        The initial capacity (number of characters).
 *
 *                          The effective string capacity will be decreased by one to reserve
 *                          space for the terminating '\0' character.
 *
 * @return  A zyan status code.
 *
 * The memory for the string is dynamically allocated by the default allocator using the default
 * growth factor of `2.0f` and the default shrink threshold of `0.25f`.
 *
 * Finalization with `ZyanStringDestroy` is required for all strings created by this function.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringInit(ZyanString* string, ZyanUSize capacity);

/**
 * @brief   Initializes the given `ZyanString` instance and sets a custom `allocator` and memory
 *          allocation/deallocation parameters.
 *
 * @param   string              A pointer to the `ZyanString` instance.
 * @param   capacity            The initial capacity (number of characters).
 *
 *                              The effective string capacity will be decreased by one to reserve
 *                              space for the terminating '\0' character.
 * @param   allocator           A pointer to a `ZyanAllocator` instance.
 * @param   growth_factor       The growth factor (from `1.0f` to `x.xf`).
 * @param   shrink_threshold    The shrink threshold (from `0.0f` to `1.0f`).
 *
 * @return  A zyan status code.
 *
 * A growth factor of `1.0f` disables overallocation and a shrink threshold of `0.0f` disables
 * dynamic shrinking.
 *
 * Finalization with `ZyanStringDestroy` is required for all strings created by this function.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringInitEx(ZyanString* string, ZyanUSize capacity,
    ZyanAllocator* allocator, float growth_factor, float shrink_threshold);

/**
 * @brief   Initializes the given `ZyanString` instance and configures it to use a custom user
 *          defined buffer with a fixed size.
 *
 * @param   string          A pointer to the `ZyanString` instance.
 * @param   buffer          A pointer to the buffer that is used as storage for the string.
 * @param   capacity        The maximum capacity (number of characters) of the buffer.
 *
 * @return  A zyan status code.
 *
 * Finalization is not required for strings created by this function.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringInitCustomBuffer(ZyanString* string, char* buffer,
    ZyanUSize capacity);

/**
 * @brief   Destroys the given `ZyanString` instance.
 *
 * @param   string  A pointer to the `ZyanString` instance.
 *
 * @return  A zyan status code.
 *
 */
ZYCORE_EXPORT ZyanStatus ZyanStringDestroy(ZyanString* string);

/* ---------------------------------------------------------------------------------------------- */
/* Duplication                                                                                    */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Initializes a new `ZyanString` instance by duplicating an existing string.
 *
 * @param   destination A pointer to the (uninitialized) destination `ZyanString` instance.
 * @param   source      A pointer to the source string.
 * @param   capacity    The initial capacity (number of characters).
 *
 *                      The effective string capacity will be decreased by one to reserve space for
 *                      the terminating '\0' character.
 *
 *                      This value is automatically adjusted to the size of the source string
 *                      - including the terminating zero -, if a smaller value was passed.
 *
 * @return  A zyan status code.
 *
 * The memory for the string is dynamically allocated by the default allocator using the default
 * growth factor of `2.0f` and the default shrink threshold of `0.25f`.
 *
 * Finalization with `ZyanStringDestroy` is required for all strings created by this function.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringDuplicate(ZyanString* destination, const ZyanString* source,
    ZyanUSize capacity);

/**
 * @brief   Initializes a new `ZyanString` instance by duplicating an existing string and sets a
 *          custom `allocator` and memory allocation/deallocation parameters.
 *
 * @param   destination         A pointer to the (uninitialized) destination `ZyanString` instance.
 * @param   source              A pointer to the source string.
 * @param   capacity            The initial capacity (number of characters).
 *
 *                              The effective string capacity will be decreased by one to reserve
 *                              space for the terminating '\0' character.
 *
 *                              This value is automatically adjusted to the size of the source
 *                              string - including the terminating zero -, if a smaller value was
 *                              passed.
 * @param   allocator           A pointer to a `ZyanAllocator` instance.
 * @param   growth_factor       The growth factor (from `1.0f` to `x.xf`).
 * @param   shrink_threshold    The shrink threshold (from `0.0f` to `1.0f`).
 *
 * @return  A zyan status code.
 *
 * A growth factor of `1.0f` disables overallocation and a shrink threshold of `0.0f` disables
 * dynamic shrinking.
 *
 * Finalization with `ZyanStringDestroy` is required for all strings created by this function.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringDuplicateEx(ZyanString* destination, const ZyanString* source,
    ZyanUSize capacity, ZyanAllocator* allocator, float growth_factor, float shrink_threshold);

/**
 * @brief   Initializes a new `ZyanString` instance by duplicating an existing string and
 *          configures it to use a custom user defined buffer with a fixed size.
 *
 * @param   destination A pointer to the (uninitialized) destination `ZyanString` instance.
 * @param   source      A pointer to the source string.
 * @param   buffer      A pointer to the buffer that is used as storage for the string.
 * @param   capacity    The maximum capacity (number of characters) of the buffer.
 *
 *                      The effective string capacity will be decreased by one to reserve space for
 *                      the terminating '\0' character.
 *
 *                      This function will fail, if the capacity of the buffer is less than the
 *                      size of the source string and the terminating zero.
 *
 * @return  A zyan status code.
 *
 * Finalization is not required for strings created by this function.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringDuplicateCustomBuffer(ZyanString* destination,
    const ZyanString* source, char* buffer, ZyanUSize capacity);

/* ---------------------------------------------------------------------------------------------- */
/* Concatenation                                                                                  */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Initializes a new `ZyanString` instance by concatenating two existing strings.
 *
 * @param   destination A pointer to the (uninitialized) destination `ZyanString` instance.
 * @param   s1          A pointer to the first source string.
 * @param   s2          A pointer to the second source string.
 * @param   capacity    The initial capacity (number of characters).
 *
 *                      The effective string capacity will be decreased by one to reserve space for
 *                      the terminating '\0' character.
 *
 *                      This value is automatically adjusted to the combined size of the source
 *                      strings - including a terminating zero -, if a smaller value was passed.
 *
 * @return  A zyan status code.
 *
 * The memory for the string is dynamically allocated by the default allocator using the default
 * growth factor of `2.0f` and the default shrink threshold of `0.25f`.
 *
 * Finalization with `ZyanStringDestroy` is required for all strings created by this function.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringConcat(ZyanString* destination, const ZyanString* s1,
    const ZyanString* s2, ZyanUSize capacity);

/**
 * @brief   Initializes a new `ZyanString` instance by concatenating two existing strings and sets
 *          a custom `allocator` and memory allocation/deallocation parameters.
 *
 * @param   destination         A pointer to the (uninitialized) destination `ZyanString` instance.
 * @param   s1                  A pointer to the first source string.
 * @param   s2                  A pointer to the second source string.
 * @param   capacity            The initial capacity (number of characters).
 *
 *                              The effective string capacity will be decreased by one to reserve
 *                              space for the terminating '\0' character.
 *
 *                              This value is automatically adjusted to the combined size of the
 *                              source strings - including a terminating zero -, if a smaller value
 *                              was passed.
 * @param   allocator           A pointer to a `ZyanAllocator` instance.
 * @param   growth_factor       The growth factor (from `1.0f` to `x.xf`).
 * @param   shrink_threshold    The shrink threshold (from `0.0f` to `1.0f`).
 *
 * @return  A zyan status code.
 *
 * A growth factor of `1.0f` disables overallocation and a shrink threshold of `0.0f` disables
 * dynamic shrinking.
 *
 * Finalization with `ZyanStringDestroy` is required for all strings created by this function.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringConcatEx(ZyanString* destination, const ZyanString* s1,
    const ZyanString* s2, ZyanUSize capacity, ZyanAllocator* allocator, float growth_factor,
    float shrink_threshold);

/**
 * @brief   Initializes a new `ZyanString` instance by concatenating two existing strings and
 *          configures it to use a custom user defined buffer with a fixed size.
 *
 * @param   destination A pointer to the (uninitialized) destination `ZyanString` instance.
 * @param   s1          A pointer to the first source string.
 * @param   s2          A pointer to the second source string.
 * @param   buffer      A pointer to the buffer that is used as storage for the string.
 * @param   capacity    The maximum capacity (number of characters) of the buffer.
 *
 *                      The effective string capacity will be decreased by one to reserve space for
 *                      the terminating '\0' character.
 *
 *                      This function will fail, if the capacity of the buffer is less than the
 *                      combined size of the source strings and the terminating zero.
 *
 * @return  A zyan status code.
 *
 * Finalization is not required for strings created by this function.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringConcatCustomBuffer(ZyanString* destination, const ZyanString* s1,
    const ZyanString* s2, char* buffer, ZyanUSize capacity);

/* ---------------------------------------------------------------------------------------------- */
/* C-String helper                                                                                */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Initializes the given `ZyanString` instance by wrapping a C-style string.
 *
 * @param   string  A pointer to the `ZyanString` instance.
 * @param   value   The C-style string.
 *
 * @return  A zyan status code.
 *
 * Strings created by this function are IMMUTABLE and do not need finalization.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringWrap(ZyanString* string, const char* value);

/**
 * @brief   Returns the C-style string of the given `ZyanString` instance.
 *
 * @param   string  A pointer to the `ZyanString` instance.
 * @param   value   Receives a pointer to the C-style string.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringUnwrap(ZyanString* string, const char** value);

/* ---------------------------------------------------------------------------------------------- */
/* Character access                                                                               */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Returns a pointer to the character at the given `index`.
 *
 * @param   string  A pointer to the `ZyanString` instance.
 * @param   index   The character index.
 * @param   value   Receives a pointer to the desired character in the string.
 *
 * @return  A zyan status code.
 *
 * This function will fail, if the `ZYAN_STRING_IS_IMMUTABLE` flag is set for the specified
 * `ZyanString` instance.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringGetCharMutable(const ZyanString* string, ZyanUSize index,
    char** value);

/**
 * @brief   Returns the character at the given `index`.
 *
 * @param   string  A pointer to the `ZyanString` instance.
 * @param   index   The character index.
 * @param   value   Receives the desired character of the string.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringGetChar(const ZyanString* string, ZyanUSize index,
    char* value);

/**
 * @brief   Assigns a new value to the character at the given `index`.
 *
 * @param   string  A pointer to the `ZyanString` instance.
 * @param   index   The character index.
 * @param   value   The character to assign.
 *
 * @return  A zyan status code.
 *
 * This function will fail, if the `ZYAN_STRING_IS_IMMUTABLE` flag is set for the specified
 * `ZyanString` instance.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringSetChar(ZyanString* string, ZyanUSize index, char value);

/* ---------------------------------------------------------------------------------------------- */
/* Insertion                                                                                      */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Inserts the content of the source string in the destination string at the given `index`.
 *
 * @param   destination The destination string.
 * @param   index       The insert index.
 * @param   source      The source string.
 *
 * @return  A zyan status code.
 *
 * This function will fail, if the `ZYAN_STRING_IS_IMMUTABLE` flag is set for the specified
 * `ZyanString` instance.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringInsert(ZyanString* destination, ZyanUSize index,
    const ZyanString* source);

/**
 * @brief   Inserts `count` characters of the source string in the destination string at the given
 *          `index`.
 *
 * @param   destination         The destination string.
 * @param   destination_index   The insert index.
 * @param   source              The source string.
 * @param   source_index        The index of the first character to be inserted from the source
 *                              string.
 * @param   count               The number of chars to insert from the source string.
 *
 * @return  A zyan status code.
 *
 * This function will fail, if the `ZYAN_STRING_IS_IMMUTABLE` flag is set for the specified
 * `ZyanString` instance.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringInsertEx(ZyanString* destination, ZyanUSize destination_index,
    const ZyanString* source, ZyanUSize source_index, ZyanUSize count);

/* ---------------------------------------------------------------------------------------------- */
/* Appending                                                                                      */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Appends the content of the source string to the end of the destination string.
 *
 * @param   destination The destination string.
 * @param   source      The source string.
 *
 * @return  A zyan status code.
 *
 * This function will fail, if the `ZYAN_STRING_IS_IMMUTABLE` flag is set for the specified
 * `ZyanString` instance.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringAppend(ZyanString* destination, const ZyanString* source);

/**
 * @brief   Appends `count` characters of the source string to the end of the destination string.
 *
 * @param   destination     The destination string.
 * @param   source          The source string.
 * @param   source_index    The index of the first character to be appended from the source string.
 * @param   count           The number of chars to append from the source string.
 *
 * @return  A zyan status code.
 *
 * This function will fail, if the `ZYAN_STRING_IS_IMMUTABLE` flag is set for the specified
 * `ZyanString` instance.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringAppendEx(ZyanString* destination, const ZyanString* source,
    ZyanUSize source_index, ZyanUSize count);

/* ---------------------------------------------------------------------------------------------- */
/* Deletion                                                                                       */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Deletes characters from the given string, starting at `index`.
 *
 * @param   string  A pointer to the `ZyanString` instance.
 * @param   index   The index of the first character to delete.
 * @param   count   The number of characters to delete.
 *
 * @return  A zyan status code.
 *
 * This function will fail, if the `ZYAN_STRING_IS_IMMUTABLE` flag is set for the specified
 * `ZyanString` instance.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringDelete(ZyanString* string, ZyanUSize index, ZyanUSize count);

/**
 * @brief   Erases the given string.
 *
 * @param   string  A pointer to the `ZyanString` instance.
 *
 * @return  A zyan status code.
 *
 * This function will fail, if the `ZYAN_STRING_IS_IMMUTABLE` flag is set for the specified
 * `ZyanString` instance.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringClear(ZyanString* string);

/* ---------------------------------------------------------------------------------------------- */
/* Searching                                                                                      */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Searches for the first occurrence of `needle` in the given `haystack` starting from the
 *          left.
 *
 * @param   haystack    The string to search in.
 * @param   needle      The sub-string to search for.
 * @param   found_index A pointer to a variable that receives the index of the first occurrence of
 *                      `needle`.
 *
 * @return  `ZYAN_STATUS_TRUE`, if the element was found, `ZYAN_STATUS_FALSE`, if not, or another
 *          zyan status code, if an error occured.
 *
 * The `found_index` is set to `-1`, if the element was not found.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringLPos(const ZyanString* haystack, const ZyanString* needle,
    ZyanISize* found_index);

/**
 * @brief   Performs a case-insensitive search for the first occurrence of `needle` in the given
 *          `haystack` starting from the left.
 *
 * @param   haystack    The string to search in.
 * @param   needle      The sub-string to search for.
 * @param   found_index A pointer to a variable that receives the index of the first occurrence of
 *                      `needle`.
 *
 * @return  `ZYAN_STATUS_TRUE`, if the element was found, `ZYAN_STATUS_FALSE`, if not, or another
 *          zyan status code, if an error occured.
 *
 * The `found_index` is set to `-1`, if the element was not found.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringLPosI(const ZyanString* haystack, const ZyanString* needle,
    ZyanISize* found_index);

/**
 * @brief   Searches for the first occurrence of `needle` in the given `haystack` starting from the
 *          left.
 *
 * @param   haystack    The string to search in.
 * @param   needle      The sub-string to search for.
 * @param   found_index A pointer to a variable that receives the index of the first occurrence of
 *                      `needle`.
 * @param   index       The start index.
 * @param   count       The maximum number of characters to iterate, beginning from the start
 *                      `index`.
 *
 * @return  `ZYAN_STATUS_TRUE`, if the element was found, `ZYAN_STATUS_FALSE`, if not, or another
 *          zyan status code, if an error occured.
 *
 * The `found_index` is set to `-1`, if the element was not found.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringLPosEx(const ZyanString* haystack, const ZyanString* needle,
    ZyanISize* found_index, ZyanUSize index, ZyanUSize count);

/**
 * @brief   Performs a case-insensitive search for the first occurrence of `needle` in the given
 *          `haystack` starting from the left.
 *
 * @param   haystack    The string to search in.
 * @param   needle      The sub-string to search for.
 * @param   found_index A pointer to a variable that receives the index of the first occurrence of
 *                      `needle`.
 * @param   index       The start index.
 * @param   count       The maximum number of characters to iterate, beginning from the start
 *                      `index`.
 *
 * @return  `ZYAN_STATUS_TRUE`, if the element was found, `ZYAN_STATUS_FALSE`, if not, or another
 *          zyan status code, if an error occured.
 *
 * The `found_index` is set to `-1`, if the element was not found.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringLPosExI(const ZyanString* haystack, const ZyanString* needle,
    ZyanISize* found_index, ZyanUSize index, ZyanUSize count);

/**
 * @brief   Searches for the first occurrence of `needle` in the given `haystack` starting from the
 *          right.
 *
 * @param   haystack    The string to search in.
 * @param   needle      The sub-string to search for.
 * @param   found_index A pointer to a variable that receives the index of the first occurrence of
 *                      `needle`.
 *
 * @return  `ZYAN_STATUS_TRUE`, if the element was found, `ZYAN_STATUS_FALSE`, if not, or another
 *          zyan status code, if an error occured.
 *
 * The `found_index` is set to `-1`, if the element was not found.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringRPos(const ZyanString* haystack, const ZyanString* needle,
    ZyanISize* found_index);

/**
 * @brief   Performs a case-insensitive search for the first occurrence of `needle` in the given
 *          `haystack` starting from the right.
 *
 * @param   haystack    The string to search in.
 * @param   needle      The sub-string to search for.
 * @param   found_index A pointer to a variable that receives the index of the first occurrence of
 *                      `needle`.
 *
 * @return  `ZYAN_STATUS_TRUE`, if the element was found, `ZYAN_STATUS_FALSE`, if not, or another
 *          zyan status code, if an error occured.
 *
 * The `found_index` is set to `-1`, if the element was not found.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringRPosI(const ZyanString* haystack, const ZyanString* needle,
    ZyanISize* found_index);

/**
 * @brief   Searches for the first occurrence of `needle` in the given `haystack` starting from the
 *          right.
 *
 * @param   haystack    The string to search in.
 * @param   needle      The sub-string to search for.
 * @param   found_index A pointer to a variable that receives the index of the first occurrence of
 *                      `needle`.
 * @param   index       The start index.
 * @param   count       The maximum number of characters to iterate, beginning from the start
 *                      `index`.
 *
 * @return  `ZYAN_STATUS_TRUE`, if the element was found, `ZYAN_STATUS_FALSE`, if not, or another
 *          zyan status code, if an error occured.
 *
 * The `found_index` is set to `-1`, if the element was not found.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringRPosEx(const ZyanString* haystack, const ZyanString* needle,
    ZyanISize* found_index, ZyanUSize index, ZyanUSize count);

/**
 * @brief   Performs a case-insensitive search for the first occurrence of `needle` in the given
 *          `haystack` starting from the right.
 *
 * @param   haystack    The string to search in.
 * @param   needle      The sub-string to search for.
 * @param   found_index A pointer to a variable that receives the index of the first occurrence of
 *                      `needle`.
 * @param   index       The start index.
 * @param   count       The maximum number of characters to iterate, beginning from the start
 *                      `index`.
 *
 * @return  `ZYAN_STATUS_TRUE`, if the element was found, `ZYAN_STATUS_FALSE`, if not, or another
 *          zyan status code, if an error occured.
 *
 * The `found_index` is set to `-1`, if the element was not found.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringRPosExI(const ZyanString* haystack, const ZyanString* needle,
    ZyanISize* found_index, ZyanUSize index, ZyanUSize count);

/* ---------------------------------------------------------------------------------------------- */
/* Comparing                                                                                      */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Compares two strings.
 *
 * @param   s1      The first string
 * @param   s2      The second string.
 * @param   result  Receives the comparison result.
 *
 *                  Values:
 *                  - `result  < 0` -> The first character that does not match has a lower value
 *                    in `s1` than in `s2`.
 *                  - `result == 0` -> The contents of both strings are equal.
 *                  - `result  > 0` -> The first character that does not match has a greater value
 *                    in `s1` than in `s2`.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringCompare(const ZyanString* s1, const ZyanString* s2,
    ZyanISize* result);

/**
 * @brief   Performs a case-insensitive comparison of two strings.
 *
 * @param   s1      The first string
 * @param   s2      The second string.
 * @param   result  Receives the comparison result.
 *
 *                  Values:
 *                  - `result  < 0` -> The first character that does not match has a lower value
 *                    in `s1` than in `s2`.
 *                  - `result == 0` -> The contents of both strings are equal.
 *                  - `result  > 0` -> The first character that does not match has a greater value
 *                    in `s1` than in `s2`.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringCompareI(const ZyanString* s1, const ZyanString* s2,
    ZyanISize* result);

/* ---------------------------------------------------------------------------------------------- */
/* Case conversion                                                                                */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Converts the given string to lowercase letters.
 *
 * @param   string      A pointer to the `ZyanString` instance.
 *
 * @return  A zyan status code.
 *
 * This function will fail, if the `ZYAN_STRING_IS_IMMUTABLE` flag is set for the specified
 * `ZyanString` instance.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringToLowerCase(ZyanString* string);

/**
 * @brief   Converts `count` characters of the given string to lowercase letters.
 *
 * @param   string  A pointer to the `ZyanString` instance.
 * @param   index   The start index.
 * @param   count   The number of characters to convert, beginning from the start `index`.
 *
 * @return  A zyan status code.
 *
 * This function will fail, if the `ZYAN_STRING_IS_IMMUTABLE` flag is set for the specified
 * `ZyanString` instance.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringToLowerCaseEx(ZyanString* string, ZyanUSize index,
    ZyanUSize count);

/**
 * @brief   Converts the given string to uppercase letters.
 *
 * @param   string      A pointer to the `ZyanString` instance.
 *
 * @return  A zyan status code.
 *
 * This function will fail, if the `ZYAN_STRING_IS_IMMUTABLE` flag is set for the specified
 * `ZyanString` instance.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringToUpperCase(ZyanString* string);

/**
 * @brief   Converts `count` characters of the given string to uppercase letters.
 *
 * @param   string  A pointer to the `ZyanString` instance.
 * @param   index   The start index.
 * @param   count   The number of characters to convert, beginning from the start `index`.
 *
 * @return  A zyan status code.
 *
 * This function will fail, if the `ZYAN_STRING_IS_IMMUTABLE` flag is set for the specified
 * `ZyanString` instance.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringToUpperCaseEx(ZyanString* string, ZyanUSize index,
    ZyanUSize count);

/* ---------------------------------------------------------------------------------------------- */
/* Memory management                                                                              */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Resizes the given `ZyanString` instance.
 *
 * @param   string  A pointer to the `ZyanString` instance.
 * @param   size    The new size of the string.
 *
 * @return  A zyan status code.
 *
 * This function will fail, if the `ZYAN_STRING_IS_IMMUTABLE` flag is set for the specified
 * `ZyanString` instance.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringResize(ZyanString* string, ZyanUSize size);

/**
 * @brief   Changes the capacity of the given `ZyanString` instance.
 *
 * @param   string      A pointer to the `ZyanString` instance.
 * @param   capacity    The new minimum capacity of the string.
 *
 * @return  A zyan status code.
 *
 * This function will fail, if the `ZYAN_STRING_IS_IMMUTABLE` flag is set for the specified
 * `ZyanString` instance.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringReserve(ZyanString* string, ZyanUSize capacity);

/**
 * @brief   Shrinks the capacity of the given string to match it's size.
 *
 * @param   string  A pointer to the `ZyanString` instance.
 *
 * @return  A zyan status code.
 *
 * This function will fail, if the `ZYAN_STRING_IS_IMMUTABLE` flag is set for the specified
 * `ZyanString` instance.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringShrinkToFit(ZyanString* string);

/* ---------------------------------------------------------------------------------------------- */
/* Information                                                                                    */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Returns the flags of the string.
 *
 * @param   string  A pointer to the `ZyanString` instance.
 * @param   flags   Receives the flags of the string.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringGetFlags(const ZyanString* string, ZyanStringFlags* flags);

/**
 * @brief   Returns the current length of the string (excluding the terminating zero character).
 *
 * @param   string  A pointer to the `ZyanString` instance.
 * @param   size    Receives the size of the string.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringGetSize(const ZyanString* string, ZyanUSize* size);

/**
 * @brief   Returns the current capacity of the string (excluding the terminating zero character).
 *
 * @param   string      A pointer to the `ZyanString` instance.
 * @param   capacity    Receives the size of the string.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanStringGetCapacity(const ZyanString* string, ZyanUSize* capacity);

/* ---------------------------------------------------------------------------------------------- */

/* ============================================================================================== */

#ifdef __cplusplus
}
#endif

#endif // ZYCORE_STRING_H
