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

#include <Zycore/API/Thread.h>
#include <Zycore/LibC.h>
#include <Zycore/Vector.h>

#ifndef ZYAN_NO_LIBC

#if defined(ZYAN_WINDOWS)
#   include <windows.h>
#   include <tlhelp32.h>
#elif defined(ZYAN_LINUX)
#   include <sys/syscall.h>
#   include <unistd.h>
#   include <dirent.h>
#   include <stdlib.h>
#elif defined(ZYAN_APPLE)
#   include <mach/mach.h>
#   include <pthread.h>
#elif defined(ZYAN_POSIX)
    // Other POSIX platforms are not supported by the thread-control API.
#else
#   error "Unsupported platform detected"
#endif

/* ============================================================================================== */
/* Enumeration                                                                                     */
/* ============================================================================================== */

ZyanStatus ZyanThreadEnumerate(ZyanVector* thread_ids, ZyanBool include_current)
{
    if (!thread_ids)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

#if defined(ZYAN_WINDOWS)

    const DWORD pid = GetCurrentProcessId();
    const DWORD self = GetCurrentThreadId();
    const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }

    THREADENTRY32 entry;
    ZYAN_MEMSET(&entry, 0, sizeof(entry));
    entry.dwSize = sizeof(entry);

    ZyanStatus status = ZYAN_STATUS_SUCCESS;
    if (Thread32First(snapshot, &entry))
    {
        do
        {
            if (entry.th32OwnerProcessID != pid)
            {
                continue;
            }
            if (!include_current && (entry.th32ThreadID == self))
            {
                continue;
            }
            const ZyanThreadId id = (ZyanThreadId)entry.th32ThreadID;
            status = ZyanVectorPushBack(thread_ids, &id);
            if (!ZYAN_SUCCESS(status))
            {
                break;
            }
        } while (Thread32Next(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return status;

#elif defined(ZYAN_LINUX)

    const pid_t self = (pid_t)syscall(SYS_gettid);
    DIR* const dir = opendir("/proc/self/task");
    if (!dir)
    {
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }

    ZyanStatus status = ZYAN_STATUS_SUCCESS;
    struct dirent* ent;
    while ((ent = readdir(dir)) != ZYAN_NULL)
    {
        if ((ent->d_name[0] == '.') &&
            ((ent->d_name[1] == '\0') || ((ent->d_name[1] == '.') && (ent->d_name[2] == '\0'))))
        {
            continue;
        }
        const long tid = strtol(ent->d_name, ZYAN_NULL, 10);
        if (tid <= 0)
        {
            continue;
        }
        if (!include_current && ((pid_t)tid == self))
        {
            continue;
        }
        const ZyanThreadId id = (ZyanThreadId)tid;
        status = ZyanVectorPushBack(thread_ids, &id);
        if (!ZYAN_SUCCESS(status))
        {
            break;
        }
    }
    closedir(dir);
    return status;

#elif defined(ZYAN_APPLE)

    const thread_act_t self = mach_thread_self();
    thread_act_array_t threads;
    mach_msg_type_number_t thread_count = 0;
    if (task_threads(mach_task_self(), &threads, &thread_count) != KERN_SUCCESS)
    {
        mach_port_deallocate(mach_task_self(), self);
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }

    ZyanStatus status = ZYAN_STATUS_SUCCESS;
    for (mach_msg_type_number_t i = 0; i < thread_count; ++i)
    {
        if (!include_current && (threads[i] == self))
        {
            mach_port_deallocate(mach_task_self(), threads[i]);
            continue;
        }
        const ZyanThreadId id = (ZyanThreadId)threads[i];
        status = ZyanVectorPushBack(thread_ids, &id);
        if (!ZYAN_SUCCESS(status))
        {
            // Release the remaining ports we will not hand back.
            for (mach_msg_type_number_t j = i; j < thread_count; ++j)
            {
                mach_port_deallocate(mach_task_self(), threads[j]);
            }
            break;
        }
    }
    mach_port_deallocate(mach_task_self(), self);
    vm_deallocate(mach_task_self(), (vm_address_t)threads, thread_count * sizeof(thread_act_t));
    return status;

#elif defined(ZYAN_POSIX)

    ZYAN_UNUSED(include_current);
    return ZYAN_STATUS_BAD_SYSTEMCALL;

#endif
}

#endif /* ZYAN_NO_LIBC */
