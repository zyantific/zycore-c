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
 * @brief
 */

#ifndef ZYCORE_THREAD_H
#define ZYCORE_THREAD_H

#include <ZycoreExportConfig.h>
#include <Zycore/Defines.h>
#include <Zycore/Status.h>

#if defined(ZYAN_POSIX)
#   include <Zycore/Internal/ThreadPosix.h>
#elif defined(ZYAN_WINDOWS)
#   include <Zycore/Internal/ThreadWindows.h>
#else
#   error "Unsupported platform detected"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================================== */
/* Enums and types                                                                                */
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

/**
 * @brief   Returns the handle of the current thread.
 *
 * @param   thread  Receives the handle of the current thread.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanThreadGetCurrentThread(ZyanThread* thread);

/**
 * @brief   Returns the unique id of the current thread.
 *
 * @param   thread_id   Receives the unique id of the current thread.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanThreadGetCurrentThreadId(ZyanThreadId* thread_id);

/* ---------------------------------------------------------------------------------------------- */
/* Thread Local Storage (TLS)                                                                     */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Allocates a new Thread Local Storage (TLS) slot.
 *
 * @param   index       Receives the TLS slot index.
 * @param   destructor  A pointer to a destructor callback which is invoked to finalize the data
 *                      in the TLS slot or `ZYAN_NULL`, if not needed.
 *
 * The maximum available number of TLS slots is implementation specific and different on each
 * platform:
 * - Windows
 *   - A total amount of 128 slots per process is guaranteed
 * - POSIX
 *   - A total amount of 128 slots per process is guaranteed
 *   - Some systems guarantee larger amounts like e.g. 1024 slots per process
 *
 * Note that the invokation rules for the destructor callback are implementation specific and
 * different on each platform:
 * - Windows
 *   - The callback is invoked when a thread exits
 *   - The callback is invoked when the process exits
 *   - The callback is invoked when the TLS slot is released
 * - POSIX
 *   - The callback is invoked when a thread exits and the stored value is not null
 *   - The callback is NOT invoked when the process exits
 *   - The callback is NOT invoked when the TLS slot is released
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanThreadTlsAlloc(ZyanThreadTlsIndex* index,
    ZyanThreadTlsCallback destructor);

/**
 * @brief   Releases a Thread Local Storage (TLS) slot.
 *
 * @param   index   The TLS slot index.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanThreadTlsFree(ZyanThreadTlsIndex index);

/**
 * @brief   Returns the value inside the given Thread Local Storage (TLS) slot for the calling
 *          thread.
 *
 * @param   index   The TLS slot index.
 * @param   data    Receives the value inside the given Thread Local Storage (TLS) slot for the
 *                  calling thread.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanThreadTlsGetValue(ZyanThreadTlsIndex index, void** data);

/**
 * @brief   Set the value of the given Thread Local Storage (TLS) slot for the calling thread.
 *
 * @param   index   The TLS slot index.
 * @param   data    The value to store inside the given Thread Local Storage (TLS) slot for the
 *                  calling thread
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanThreadTlsSetValue(ZyanThreadTlsIndex index, void* data);

/* ---------------------------------------------------------------------------------------------- */

/* ============================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* ZYCORE_THREAD_H */
