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

#include <Zycore/String.h>
#include "Zycore/LibC.h"

/* ============================================================================================== */
/* Internal macros                                                                                */
/* ============================================================================================== */

/**
 * @brief   Writes a terminating '\0' character at the end of the string data.
 */
#define ZYCORE_STRING_NULLTERMINATE(string) \
      *(char*)((ZyanU8*)(string)->data.data + (string)->data.size - 1) = '\0';

/**
 * @brief   Checks for a terminating '\0' character at the end of the string data.
 */
#define ZYCORE_STRING_ASSERT_NULLTERMINATION(string) \
      ZYAN_ASSERT(*(char*)((ZyanU8*)(string)->data.data + (string)->data.size - 1) == '\0');

/* ============================================================================================== */
/* Internal functions                                                                             */
/* ============================================================================================== */

/* ---------------------------------------------------------------------------------------------- */
/*                                                                                                */
/* ---------------------------------------------------------------------------------------------- */



/* ---------------------------------------------------------------------------------------------- */

/* ============================================================================================== */
/* Exported functions                                                                             */
/* ============================================================================================== */

/* ---------------------------------------------------------------------------------------------- */
/* Constructor and destructor                                                                     */
/* ---------------------------------------------------------------------------------------------- */

ZyanStatus ZyanStringInit(ZyanString* string, ZyanUSize capacity)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    string->flags = 0;
    ZYAN_CHECK(ZyanVectorInit(&string->data, sizeof(char), capacity + 1));
    // Some of the string code relies on `sizeof(char) == 1`
    ZYAN_ASSERT(string->data.element_size == 1);
    // `ZyanVector` guarantees a minimum capacity of 1 element/character
    ZYAN_ASSERT(string->data.capacity >= 1);

    *(char*)string->data.data = '\0';
    ++string->data.size;

    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanStringInitEx(ZyanString* string, ZyanUSize capacity, ZyanAllocator* allocator,
    float growth_factor, float shrink_threshold)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    string->flags = 0;
    ZYAN_CHECK(ZyanVectorInitEx(&string->data, sizeof(char), capacity + 1, allocator, growth_factor,
         shrink_threshold));
    // Some of the string code relies on `sizeof(char) == 1`
    ZYAN_ASSERT(string->data.element_size == 1);
    // `ZyanVector` guarantees a minimum capacity of 1 element/character
    ZYAN_ASSERT(string->data.capacity >= 1);

    *(char*)string->data.data = '\0';
    ++string->data.size;

    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanStringInitCustomBuffer(ZyanString* string, char* buffer, ZyanUSize capacity)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    string->flags = ZYAN_STRING_HAS_FIXED_CAPACITY;;
    ZYAN_CHECK(ZyanVectorInitCustomBuffer(&string->data, sizeof(char), (void*)buffer, capacity));
    // Some of the string code relies on `sizeof(char) == 1`
    ZYAN_ASSERT(string->data.element_size == 1);
    // `ZyanVector` guarantees a minimum capacity of 1 element/character
    ZYAN_ASSERT(string->data.capacity >= 1);

    *(char*)string->data.data = '\0';
    ++string->data.size;

    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanStringDestroy(ZyanString* string)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }
    if (string->flags & ZYAN_STRING_IS_IMMUTABLE)
    {
        return ZYAN_STATUS_SUCCESS;
    }

    return ZyanVectorDestroy(&string->data);
}

/* ---------------------------------------------------------------------------------------------- */
/* Duplication                                                                                    */
/* ---------------------------------------------------------------------------------------------- */

ZyanStatus ZyanStringDuplicate(ZyanString* destination, const ZyanString* source,
    ZyanUSize capacity)
{
    if (!source)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    ZYAN_ASSERT(source->data.size >= 1);
    ZYCORE_STRING_ASSERT_NULLTERMINATION(source);
    const ZyanUSize len = source->data.size;

    capacity = ZYAN_MAX(capacity + 1, len);
    ZYAN_CHECK(ZyanStringInit(destination, capacity));
    ZYAN_ASSERT(destination->data.capacity >= len);

    ZYAN_MEMCPY(destination->data.data, source->data.data, source->data.size);
    destination->data.size = len;
    ZYCORE_STRING_ASSERT_NULLTERMINATION(destination);

    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanStringDuplicateEx(ZyanString* destination, const ZyanString* source,
    ZyanUSize capacity, ZyanAllocator* allocator, float growth_factor, float shrink_threshold)
{
    if (!source)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    ZYAN_ASSERT(source->data.size >= 1);
    ZYCORE_STRING_ASSERT_NULLTERMINATION(source);
    const ZyanUSize len = source->data.size;

    capacity = ZYAN_MAX(capacity + 1, len);
    ZYAN_CHECK(ZyanStringInitEx(destination, capacity, allocator, growth_factor, shrink_threshold));
    ZYAN_ASSERT(destination->data.capacity >= len);

    ZYAN_MEMCPY(destination->data.data, source->data.data, source->data.size);
    destination->data.size = len;
    ZYCORE_STRING_ASSERT_NULLTERMINATION(destination);

    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanStringDuplicateCustomBuffer(ZyanString* destination, const ZyanString* source,
    char* buffer, ZyanUSize capacity)
{
    if (!source)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    ZYAN_ASSERT(source->data.size >= 1);
    ZYCORE_STRING_ASSERT_NULLTERMINATION(source);
    const ZyanUSize len = source->data.size;

    if (capacity < len)
    {
        return ZYAN_STATUS_INSUFFICIENT_BUFFER_SIZE;
    }

    ZYAN_CHECK(ZyanStringInitCustomBuffer(destination, buffer, capacity));
    ZYAN_ASSERT(destination->data.capacity >= len);

    ZYAN_MEMCPY(destination->data.data, source->data.data, source->data.size);
    destination->data.size = len;
    ZYCORE_STRING_ASSERT_NULLTERMINATION(destination);

    return ZYAN_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------------------------- */
/* Concatenation                                                                                  */
/* ---------------------------------------------------------------------------------------------- */

ZyanStatus ZyanStringConcat(ZyanString* destination, const ZyanString* s1, const ZyanString* s2,
    ZyanUSize capacity)
{
    if (!s1 || !s2)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    ZYAN_ASSERT(s1->data.size >= 1);
    ZYAN_ASSERT(s2->data.size >= 1);
    ZYCORE_STRING_ASSERT_NULLTERMINATION(s1);
    ZYCORE_STRING_ASSERT_NULLTERMINATION(s2);
    const ZyanUSize len = s1->data.size + s2->data.size - 1;

    capacity = ZYAN_MAX(capacity + 1, len);
    ZYAN_CHECK(ZyanStringInit(destination, capacity));
    ZYAN_ASSERT(destination->data.capacity >= len);

    ZYAN_MEMCPY(destination->data.data, s1->data.data, s1->data.size - 1);
    ZYAN_MEMCPY((char*)destination->data.data + s1->data.size - 1, s2->data.data, s2->data.size);
    destination->data.size = len;
    ZYCORE_STRING_ASSERT_NULLTERMINATION(destination);

    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanStringConcatEx(ZyanString* destination, const ZyanString* s1, const ZyanString* s2,
    ZyanUSize capacity, ZyanAllocator* allocator, float growth_factor, float shrink_threshold)
{
    if (!s1 || !s2)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    ZYAN_ASSERT(s1->data.size >= 1);
    ZYAN_ASSERT(s2->data.size >= 1);
    ZYCORE_STRING_ASSERT_NULLTERMINATION(s1);
    ZYCORE_STRING_ASSERT_NULLTERMINATION(s2);
    const ZyanUSize len = s1->data.size + s2->data.size - 1;

    capacity = ZYAN_MAX(capacity + 1, len);
    ZYAN_CHECK(ZyanStringInitEx(destination, capacity, allocator, growth_factor, shrink_threshold));
    ZYAN_ASSERT(destination->data.capacity >= len);

    ZYAN_MEMCPY(destination->data.data, s1->data.data, s1->data.size - 1);
    ZYAN_MEMCPY((char*)destination->data.data + s1->data.size - 1, s2->data.data, s2->data.size);
    destination->data.size = len;
    ZYCORE_STRING_ASSERT_NULLTERMINATION(destination);

    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanStringConcatCustomBuffer(ZyanString* destination, const ZyanString* s1,
    const ZyanString* s2, char* buffer, ZyanUSize capacity)
{
    if (!s1 || !s2)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    ZYAN_ASSERT(s1->data.size >= 1);
    ZYAN_ASSERT(s2->data.size >= 1);
    ZYCORE_STRING_ASSERT_NULLTERMINATION(s1);
    ZYCORE_STRING_ASSERT_NULLTERMINATION(s2);
    const ZyanUSize len = s1->data.size + s2->data.size - 1;

    if (capacity < len)
    {
        return ZYAN_STATUS_INSUFFICIENT_BUFFER_SIZE;
    }

    ZYAN_CHECK(ZyanStringInitCustomBuffer(destination, buffer, capacity));
    ZYAN_ASSERT(destination->data.capacity >= len);

    ZYAN_MEMCPY(destination->data.data, s1->data.data, s1->data.size - 1);
    ZYAN_MEMCPY((char*)destination->data.data + s1->data.size - 1, s2->data.data, s2->data.size);
    destination->data.size = len;
    ZYCORE_STRING_ASSERT_NULLTERMINATION(destination);

    return ZYAN_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------------------------- */
/* C-String helper                                                                                */
/* ---------------------------------------------------------------------------------------------- */

ZyanStatus ZyanStringWrap(ZyanString* string, const char* value)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    string->flags                   = ZYAN_STRING_IS_IMMUTABLE;
    string->data.allocator          = ZYAN_NULL;
    string->data.growth_factor      = 1.0f;
    string->data.shrink_threshold   = 0.0f;
    string->data.size               = ZYAN_STRLEN(value) + 1;
    string->data.capacity           = string->data.size;
    string->data.element_size       = sizeof(char);
    string->data.data               = (void*)value;

    // Some of the string code relies on `sizeof(char) == 1`
    ZYAN_ASSERT(string->data.element_size == 1);

    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanStringUnwrap(ZyanString* string, const char** value)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    *value = string->data.data;

    return ZYAN_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------------------------- */
/* Character access                                                                               */
/* ---------------------------------------------------------------------------------------------- */

ZyanStatus ZyanStringGetCharMutable(const ZyanString* string, ZyanUSize index, char** value)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    if (string->flags & ZYAN_STRING_IS_IMMUTABLE)
    {
        return ZYAN_STATUS_INVALID_OPERATION;
    }

    // Don't allow direct access to the terminating '\0' character
    if (index + 1 >= string->data.size)
    {
        return ZYAN_STATUS_OUT_OF_RANGE;
    }

    return ZyanVectorGetElementMutable(&string->data, index, (void**)value);
}

ZyanStatus ZyanStringGetChar(const ZyanString* string, ZyanUSize index, char* value)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    // Don't allow direct access to the terminating '\0' character
    if (index + 1 >= string->data.size)
    {
        return ZYAN_STATUS_OUT_OF_RANGE;
    }

    const char* c;
    ZYAN_CHECK(ZyanVectorGetElement(&string->data, index, (const void**)&c));
    *value = *c;

    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanStringSetChar(ZyanString* string, ZyanUSize index, char value)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    if (string->flags & ZYAN_STRING_IS_IMMUTABLE)
    {
        return ZYAN_STATUS_INVALID_OPERATION;
    }

    // Don't allow direct access to the terminating '\0' character
    if (index + 1 >= string->data.size)
    {
        return ZYAN_STATUS_OUT_OF_RANGE;
    }

    return ZyanVectorSetElement(&string->data, index, (void*)&value);
}

/* ---------------------------------------------------------------------------------------------- */
/* Insertion                                                                                      */
/* ---------------------------------------------------------------------------------------------- */

ZyanStatus ZyanStringInsert(ZyanString* destination, ZyanUSize index, const ZyanString* source)
{
    if (!destination || !source)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    if (index == destination->data.size)
    {
        return ZyanStringAppend(destination, source);
    }

    if (destination->flags & ZYAN_STRING_IS_IMMUTABLE)
    {
        return ZYAN_STATUS_INVALID_OPERATION;
    }

    // Don't allow insertion after the terminating '\0' character
    if (index >= destination->data.size)
    {
        return ZYAN_STATUS_OUT_OF_RANGE;
    }

    ZYAN_CHECK(ZyanVectorInsertEx(&destination->data, index, source->data.data,
        source->data.size - 1));
    ZYCORE_STRING_ASSERT_NULLTERMINATION(destination);

    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanStringInsertEx(ZyanString* destination, ZyanUSize destination_index,
    const ZyanString* source, ZyanUSize source_index, ZyanUSize count)
{
    if (!destination || !source)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    if (destination_index == destination->data.size)
    {
        return ZyanStringAppendEx(destination, source, source_index, count);
    }

    if (destination->flags & ZYAN_STRING_IS_IMMUTABLE)
    {
        return ZYAN_STATUS_INVALID_OPERATION;
    }

    // Don't allow insertion after the terminating '\0' character
    if (destination_index >= destination->data.size)
    {
        return ZYAN_STATUS_OUT_OF_RANGE;
    }

    // Don't allow access to the terminating '\0' character
    if (source_index + count >= source->data.size)
    {
        return ZYAN_STATUS_OUT_OF_RANGE;
    }

    ZYAN_CHECK(ZyanVectorInsertEx(&destination->data, destination_index,
        (char*)source->data.data + source_index, count));
    ZYCORE_STRING_ASSERT_NULLTERMINATION(destination);

    return ZYAN_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------------------------- */
/* Appending                                                                                      */
/* ---------------------------------------------------------------------------------------------- */

ZyanStatus ZyanStringAppend(ZyanString* destination, const ZyanString* source)
{
    if (!destination || !source)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    if (destination->flags & ZYAN_STRING_IS_IMMUTABLE)
    {
        return ZYAN_STATUS_INVALID_OPERATION;
    }

    // Decrease the size of the vector to avoid shifting and copy the terminating '\0' from the
    // source string instead
    ZYAN_CHECK(ZyanVectorInsertEx(&destination->data, --destination->data.size, source->data.data,
        source->data.size));
    ZYCORE_STRING_ASSERT_NULLTERMINATION(destination);

    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanStringAppendEx(ZyanString* destination, const ZyanString* source,
    ZyanUSize source_index, ZyanUSize count)
{
    if (!destination || !source)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    if (destination->flags & ZYAN_STRING_IS_IMMUTABLE)
    {
        return ZYAN_STATUS_INVALID_OPERATION;
    }

    // Don't allow access to the terminating '\0' character
    if (source_index + count >= source->data.size)
    {
        return ZYAN_STATUS_OUT_OF_RANGE;
    }

    // Decrease the size of the vector to avoid shifting and add the terminating '\0' later. We
    // insert one additional character from the source stream to make sure we have enough space.
    ZYAN_CHECK(ZyanVectorInsertEx(&destination->data, --destination->data.size,
        (char*)source->data.data + source_index, count + 1));
    ZYCORE_STRING_NULLTERMINATE(destination);

    return ZYAN_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------------------------- */
/* Deletion                                                                                       */
/* ---------------------------------------------------------------------------------------------- */

ZyanStatus ZyanStringDelete(ZyanString* string, ZyanUSize index, ZyanUSize count)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    if (string->flags & ZYAN_STRING_IS_IMMUTABLE)
    {
        return ZYAN_STATUS_INVALID_OPERATION;
    }

    // Don't allow removal of the terminating '\0' character
    if (index + count >= string->data.size)
    {
        return ZYAN_STATUS_OUT_OF_RANGE;
    }

    ZYAN_CHECK(ZyanVectorDeleteEx(&string->data, index, count));
    ZYCORE_STRING_NULLTERMINATE(string);

    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanStringClear(ZyanString* string)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    if (string->flags & ZYAN_STRING_IS_IMMUTABLE)
    {
        return ZYAN_STATUS_INVALID_OPERATION;
    }

    ZYAN_CHECK(ZyanVectorClear(&string->data));
    // `ZyanVector` guarantees a minimum capacity of 1 element/character
    ZYAN_ASSERT(string->data.capacity >= 1);

    *(char*)string->data.data = '\0';
    string->data.size++;

    return ZYAN_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------------------------- */
/* Searching                                                                                      */
/* ---------------------------------------------------------------------------------------------- */

ZyanStatus ZyanStringLPos(const ZyanString* haystack, const ZyanString* needle,
    ZyanISize* found_index)
{
    if (!haystack)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    return ZyanStringLPosEx(haystack, needle, found_index, 0, haystack->data.size - 1);
}

ZyanStatus ZyanStringLPosEx(const ZyanString* haystack, const ZyanString* needle,
    ZyanISize* found_index, ZyanUSize index, ZyanUSize count)
{
    if (!haystack || !needle || !found_index)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    // Don't allow access to the terminating '\0' character
    if (index + count >= haystack->data.size)
    {
        return ZYAN_STATUS_OUT_OF_RANGE;
    }

    if ((haystack->data.size == 1) || (needle->data.size == 1) ||
        (haystack->data.size < needle->data.size))
    {
        *found_index = -1;
        return ZYAN_STATUS_FALSE;
    }

    const char* s = (char*)haystack->data.data + index;
    const char* b = (char*)needle->data.data;
    for (; s != 0; ++s)
    {
        if (*s != *b)
        {
            continue;
        }
        const char* a = s;
        for (;;)
        {
            if ((ZyanUSize)(a - (char*)haystack->data.data) > index + count)
            {
                *found_index = -1;
                return ZYAN_STATUS_FALSE;
            }
            if (*b == 0)
            {
                *found_index = (ZyanISize)(s - (char*)haystack->data.data);
                return ZYAN_STATUS_TRUE;
            }
            if (*a++ != *b++)
            {
                break;
            }
        }
        b = (char*)needle->data.data;
    }

    return ZYAN_STATUS_FALSE;
}

ZyanStatus ZyanStringLPosI(const ZyanString* haystack, const ZyanString* needle,
    ZyanISize* found_index)
{
    if (!haystack)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    return ZyanStringLPosIEx(haystack, needle, found_index, 0, haystack->data.size - 1);
}

ZyanStatus ZyanStringLPosIEx(const ZyanString* haystack, const ZyanString* needle,
    ZyanISize* found_index, ZyanUSize index, ZyanUSize count)
{
    // This solution assumes that characters are represented using ASCII representation, i.e.,
    // codes for 'a', 'b', 'c', .. 'z' are 97, 98, 99, .. 122 respectively. And codes for 'A',
    // 'B', 'C', .. 'Z' are 65, 66, .. 95 respectively.

    if (!haystack || !needle || !found_index)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    // Don't allow access to the terminating '\0' character
    if (index + count >= haystack->data.size)
    {
        return ZYAN_STATUS_OUT_OF_RANGE;
    }

    if ((haystack->data.size == 1) || (needle->data.size == 1) ||
        (haystack->data.size < needle->data.size))
    {
        *found_index = -1;
        return ZYAN_STATUS_FALSE;
    }

    const char* s = (char*)haystack->data.data + index;
    const char* b = (char*)needle->data.data;
    for (; s != 0; ++s)
    {
        if ((*s != *b) && ((*s ^ 32) != *b))
        {
            continue;
        }
        const char* a = s;
        for (;;)
        {
            if ((ZyanUSize)(a - (char*)haystack->data.data) > index + count)
            {
                *found_index = -1;
                return ZYAN_STATUS_FALSE;
            }
            if (*b == 0)
            {
                *found_index = (ZyanISize)(s - (char*)haystack->data.data);
                return ZYAN_STATUS_TRUE;
            }
            const char c1 = *a++;
            const char c2 = *b++;
            if ((c1 != c2) && ((c1 ^ 32) != c2))
            {
                break;
            }
        }
        b = (char*)needle->data.data;
    }

    return ZYAN_STATUS_FALSE;
}

ZyanStatus ZyanStringRPos(const ZyanString* haystack, const ZyanString* needle,
    ZyanISize* found_index)
{
    ZYAN_UNUSED(haystack);
    ZYAN_UNUSED(needle);
    ZYAN_UNUSED(found_index);
    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanStringRPosEx(const ZyanString* haystack, const ZyanString* needle,
    ZyanISize* found_index, ZyanUSize index, ZyanUSize count)
{
    ZYAN_UNUSED(haystack);
    ZYAN_UNUSED(needle);
    ZYAN_UNUSED(found_index);
    ZYAN_UNUSED(index);
    ZYAN_UNUSED(count);
    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanStringRPosI(const ZyanString* haystack, const ZyanString* needle,
    ZyanISize* found_index)
{
    ZYAN_UNUSED(haystack);
    ZYAN_UNUSED(needle);
    ZYAN_UNUSED(found_index);
    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanStringRPosIEx(const ZyanString* haystack, const ZyanString* needle,
    ZyanISize* found_index, ZyanUSize index, ZyanUSize count)
{
    ZYAN_UNUSED(haystack);
    ZYAN_UNUSED(needle);
    ZYAN_UNUSED(found_index);
    ZYAN_UNUSED(index);
    ZYAN_UNUSED(count);
    return ZYAN_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------------------------- */
/* Comparing                                                                                      */
/* ---------------------------------------------------------------------------------------------- */

ZyanStatus ZyanStringCompare(const ZyanString* s1, const ZyanString* s2, ZyanI32* result)
{
    if (!s1 || !s2)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    if (s1->data.size < s2->data.size)
    {
        *result = -1;
        return ZYAN_STATUS_FALSE;
    }
    if (s1->data.size > s2->data.size)
    {
        *result =  1;
        return ZYAN_STATUS_FALSE;
    }

    *result = ZYAN_STRCMP(s1->data.data, s2->data.data);
    if (*result == 0)
    {
        return ZYAN_STATUS_TRUE;
    }

    return ZYAN_STATUS_FALSE;
}

ZyanStatus ZyanStringCompareI(const ZyanString* s1, const ZyanString* s2, ZyanI32* result)
{
    // This solution assumes that characters are represented using ASCII representation, i.e.,
    // codes for 'a', 'b', 'c', .. 'z' are 97, 98, 99, .. 122 respectively. And codes for 'A',
    // 'B', 'C', .. 'Z' are 65, 66, .. 95 respectively.

    if (!s1 || !s2)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    if (s1->data.size < s2->data.size)
    {
        *result = -1;
        return ZYAN_STATUS_FALSE;
    }
    if (s1->data.size > s2->data.size)
    {
        *result =  1;
        return ZYAN_STATUS_FALSE;
    }

    const char* const a = (char*)s1->data.data;
    const char* const b = (char*)s2->data.data;
    ZyanUSize i;
    for (i = 0; a[i] && b[i]; ++i)
    {
        if ((a[i] == b[i]) || ((a[i] ^ 32) == b[i]))
        {
            continue;
        }
        break;
    }

    if (a[i] == b[i])
    {
        *result = 0;
        return ZYAN_STATUS_TRUE;
    }

    if ((a[i] | 32) < (b[i] | 32))
    {
        *result = -1;
        return ZYAN_STATUS_FALSE;
    }

    *result = 1;
    return ZYAN_STATUS_FALSE;
}

/* ---------------------------------------------------------------------------------------------- */
/* Case conversion                                                                                */
/* ---------------------------------------------------------------------------------------------- */

ZyanStatus ZyanStringToLowerCase(ZyanString* string)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    return ZyanStringToLowerCaseEx(string, 0, string->data.size - 1);
}

ZyanStatus ZyanStringToLowerCaseEx(ZyanString* string, ZyanUSize index, ZyanUSize count)
{
    // This solution assumes that characters are represented using ASCII representation, i.e.,
    // codes for 'a', 'b', 'c', .. 'z' are 97, 98, 99, .. 122 respectively. And codes for 'A',
    // 'B', 'C', .. 'Z' are 65, 66, .. 95 respectively.

    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    if (string->flags & ZYAN_STRING_IS_IMMUTABLE)
    {
        return ZYAN_STATUS_INVALID_OPERATION;
    }

    // Don't allow access to the terminating '\0' character
    if (index + count >= string->data.size)
    {
        return ZYAN_STATUS_OUT_OF_RANGE;
    }

    char* s = (char*)string->data.data + index;
    for (ZyanUSize i = index; i < index + count; ++i)
    {
        const char c = *s;
        if ((c >= 'A') && (c <= 'Z'))
        {
            *s = c | 32;
        }
        ++s;
    }

    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanStringToUpperCase(ZyanString* string)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    return ZyanStringToUpperCaseEx(string, 0, string->data.size - 1);
}

ZyanStatus ZyanStringToUpperCaseEx(ZyanString* string, ZyanUSize index, ZyanUSize count)
{
    // This solution assumes that characters are represented using ASCII representation, i.e.,
    // codes for 'a', 'b', 'c', .. 'z' are 97, 98, 99, .. 122 respectively. And codes for 'A',
    // 'B', 'C', .. 'Z' are 65, 66, .. 95 respectively.

    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    if (string->flags & ZYAN_STRING_IS_IMMUTABLE)
    {
        return ZYAN_STATUS_INVALID_OPERATION;
    }

    // Don't allow access to the terminating '\0' character
    if (index + count >= string->data.size)
    {
        return ZYAN_STATUS_OUT_OF_RANGE;
    }

    char* s = (char*)string->data.data + index;
    for (ZyanUSize i = index; i < index + count; ++i)
    {
        const char c = *s;
        if ((c >= 'a') && (c <= 'z'))
        {
            *s = c & ~32;
        }
        ++s;
    }

    return ZYAN_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------------------------- */
/* Memory management                                                                              */
/* ---------------------------------------------------------------------------------------------- */

ZyanStatus ZyanStringResize(ZyanString* string, ZyanUSize size)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    if (string->flags & ZYAN_STRING_IS_IMMUTABLE)
    {
        return ZYAN_STATUS_INVALID_OPERATION;
    }

    ZYAN_CHECK(ZyanVectorResize(&string->data, size + 1));
    ZYCORE_STRING_NULLTERMINATE(string);

    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanStringReserve(ZyanString* string, ZyanUSize capacity)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    if (string->flags & ZYAN_STRING_IS_IMMUTABLE)
    {
        return ZYAN_STATUS_INVALID_OPERATION;
    }

    return ZyanVectorReserve(&string->data, capacity);
}

ZyanStatus ZyanStringShrinkToFit(ZyanString* string)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    if (string->flags & ZYAN_STRING_IS_IMMUTABLE)
    {
        return ZYAN_STATUS_INVALID_OPERATION;
    }

    return ZyanVectorShrinkToFit(&string->data);
}

/* ---------------------------------------------------------------------------------------------- */
/* Information                                                                                    */
/* ---------------------------------------------------------------------------------------------- */

ZyanStatus ZyanStringGetFlags(const ZyanString* string, ZyanStringFlags* flags)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    *flags = string->flags;

    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanStringGetSize(const ZyanString* string, ZyanUSize* size)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    ZYAN_ASSERT(string->data.size >= 1);
    *size = string->data.size - 1;

    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanStringGetCapacity(const ZyanString* string, ZyanUSize* capacity)
{
    if (!string)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    ZYAN_ASSERT(string->data.capacity >= 1);
    *capacity = string->data.capacity - 1;

    return ZYAN_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------------------------- */

/* ============================================================================================== */
