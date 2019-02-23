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

#include <Zycore/Defines.h>

#if defined(ZYAN_WINDOWS)

#include <Zycore/Thread.h>
#include <Zycore/Internal/ThreadWindows.h>

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
/* General                                                                                        */
/* ---------------------------------------------------------------------------------------------- */

ZyanStatus ZyanThreadGetCurrentThread(ZyanThread* thread)
{
    *thread = GetCurrentThread();

    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanThreadGetCurrentThreadId(ZyanThreadId* thread_id)
{
    *thread_id = GetCurrentThreadId();

    return ZYAN_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------------------------- */
/* Thread Local Storage (TLS)                                                                     */
/* ---------------------------------------------------------------------------------------------- */

ZyanStatus ZyanThreadTlsAlloc(ZyanThreadTlsIndex* index, ZyanThreadTlsCallback destructor)
{
    const ZyanThreadTlsIndex value = FlsAlloc(destructor);
    if (value == FLS_OUT_OF_INDEXES)
    {
        return ZYAN_STATUS_OUT_OF_RESOURCES;
    }

    *index = value;
    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanThreadTlsFree(ZyanThreadTlsIndex index)
{
    return FlsFree(index) ? ZYAN_STATUS_SUCCESS : ZYAN_STATUS_BAD_SYSTEMCALL;
}

ZyanStatus ZyanThreadTlsGetValue(ZyanThreadTlsIndex index, void** data)
{
    *data = FlsGetValue(index);

    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanThreadTlsSetValue(ZyanThreadTlsIndex index, void* data)
{
    if (!FlsSetValue(index, data))
    {
        const DWORD error = GetLastError();
        if (error == ERROR_INVALID_PARAMETER)
        {
            return ZYAN_STATUS_INVALID_ARGUMENT;
        }
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }

    return ZYAN_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------------------------- */

/* ============================================================================================== */

#endif
