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

#include <Zycore/API/Synchronization.h>

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

#if   defined(ZYAN_POSIX)

/* ---------------------------------------------------------------------------------------------- */
/* Critical Section                                                                               */
/* ---------------------------------------------------------------------------------------------- */

void ZyanCriticalSectionInitialize(ZyanCriticalSection* critical_section)
{
    pthread_mutexattr_t attribute;

    pthread_mutexattr_init(&attribute);
    pthread_mutexattr_settype(&attribute, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(critical_section, &attribute);
    pthread_mutexattr_destroy(&attribute);
}

void ZyanCriticalSectionEnter(ZyanCriticalSection* critical_section)
{
    pthread_mutex_lock(critical_section);
}

ZyanBool ZyanCriticalSectionTryEnter(ZyanCriticalSection* critical_section)
{
    return pthread_mutex_trylock(critical_section) ? ZYAN_FALSE : ZYAN_TRUE;
}

void ZyanCriticalSectionLeave(ZyanCriticalSection* critical_section)
{
    pthread_mutex_unlock(critical_section);
}

void ZyanCriticalSectionDelete(ZyanCriticalSection* critical_section)
{
    pthread_mutex_destroy(critical_section);
}

/* ---------------------------------------------------------------------------------------------- */

#elif defined(ZYAN_WINDOWS)

/* ---------------------------------------------------------------------------------------------- */
/* General                                                                                        */
/* ---------------------------------------------------------------------------------------------- */

void ZyanCriticalSectionInitialize(ZyanCriticalSection* critical_section)
{
    InitializeCriticalSection(critical_section);
}

void ZyanCriticalSectionEnter(ZyanCriticalSection* critical_section)
{
    EnterCriticalSection(critical_section);
}

ZyanBool ZyanCriticalSectionTryEnter(ZyanCriticalSection* critical_section)
{
    return TryEnterCriticalSection(critical_section) ? ZYAN_TRUE : ZYAN_FALSE;
}

void ZyanCriticalSectionLeave(ZyanCriticalSection* critical_section)
{
    LeaveCriticalSection(critical_section);
}

void ZyanCriticalSectionDelete(ZyanCriticalSection* critical_section)
{
    DeleteCriticalSection(critical_section);
}

/* ---------------------------------------------------------------------------------------------- */

#else
#   error "Unsupported platform detected"
#endif

/* ============================================================================================== */
