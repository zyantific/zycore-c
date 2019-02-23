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

#ifndef ZYCORE_THREAD_POSIX_H
#define ZYCORE_THREAD_POSIX_H

#include <Zycore/Defines.h>

#if defined(ZYAN_POSIX)

#include <pthread.h>
#include <Zycore/Status.h>
#include <Zycore/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================================== */
/* Enums and types                                                                                */
/* ============================================================================================== */

/* ---------------------------------------------------------------------------------------------- */
/* General                                                                                        */
/* ---------------------------------------------------------------------------------------------- */

/**
 *  @brief  Defines the `ZyanThread` datatype.
 */
typedef pthread_t ZyanThread;

/**
 *  @brief  Defines the `ZyanThreadId` datatype.
 */
typedef ZyanU64 ZyanThreadId;

/* ---------------------------------------------------------------------------------------------- */
/* Thread Local Storage (TLS)                                                                     */
/* ---------------------------------------------------------------------------------------------- */

/**
 *  @brief  Defines the `ZyanThreadTlsIndex` datatype.
 */
typedef pthread_key_t ZyanThreadTlsIndex;

/**
 *  @brief  Defines the `ZyanThreadTlsCallback` function prototype.
 */
typedef void(*ZyanThreadTlsCallback)(void* data);

/**
 * @brief   Declares a Thread Local Storage (TLS) callback function.
 *
 * @param   name    The callback function name.
 * @param   data    The callback data parameter name.
 */
#define ZYAN_THREAD_DECLARE_TLS_CALLBACK(name, data) \
    void name(void* data)

/* ---------------------------------------------------------------------------------------------- */

/* ============================================================================================== */

#ifdef __cplusplus
}
#endif

#endif

#endif /* ZYCORE_THREAD_POSIX_H */
