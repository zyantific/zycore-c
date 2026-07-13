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
#   include <signal.h>
#   include <ucontext.h>
#   include <semaphore.h>
#   include <string.h>
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

/* ============================================================================================== */
/* Suspend / resume / instruction pointer                                                         */
/* ============================================================================================== */

#if defined(ZYAN_WINDOWS)

ZyanStatus ZyanThreadSuspend(ZyanThreadId thread_id)
{
    const HANDLE handle = OpenThread(THREAD_SUSPEND_RESUME, ZYAN_FALSE, (DWORD)thread_id);
    if (!handle)
    {
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }
    const DWORD result = SuspendThread(handle);
    CloseHandle(handle);
    return (result == (DWORD)-1) ? ZYAN_STATUS_BAD_SYSTEMCALL : ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanThreadResume(ZyanThreadId thread_id)
{
    const HANDLE handle = OpenThread(THREAD_SUSPEND_RESUME, ZYAN_FALSE, (DWORD)thread_id);
    if (!handle)
    {
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }
    const DWORD result = ResumeThread(handle);
    CloseHandle(handle);
    return (result == (DWORD)-1) ? ZYAN_STATUS_BAD_SYSTEMCALL : ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanThreadGetInstructionPointer(ZyanThreadId thread_id, ZyanUPointer* ip)
{
    if (!ip)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }
#if defined(ZYAN_X64) || defined(ZYAN_X86)
    const HANDLE handle = OpenThread(THREAD_GET_CONTEXT, ZYAN_FALSE, (DWORD)thread_id);
    if (!handle)
    {
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }
    CONTEXT context;
    ZYAN_MEMSET(&context, 0, sizeof(context));
    context.ContextFlags = CONTEXT_CONTROL;
    const BOOL ok = GetThreadContext(handle, &context);
    CloseHandle(handle);
    if (!ok)
    {
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }
#   if defined(ZYAN_X64)
    *ip = (ZyanUPointer)context.Rip;
#   else
    *ip = (ZyanUPointer)context.Eip;
#   endif
    return ZYAN_STATUS_SUCCESS;
#else
    // Register access is only implemented for x86-64 (and x86) this round.
    ZYAN_UNUSED(thread_id);
    ZYAN_UNUSED(ip);
    return ZYAN_STATUS_BAD_SYSTEMCALL;
#endif
}

ZyanStatus ZyanThreadSetInstructionPointer(ZyanThreadId thread_id, ZyanUPointer ip)
{
#if defined(ZYAN_X64) || defined(ZYAN_X86)
    const HANDLE handle = OpenThread(THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, ZYAN_FALSE,
        (DWORD)thread_id);
    if (!handle)
    {
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }
    CONTEXT context;
    ZYAN_MEMSET(&context, 0, sizeof(context));
    context.ContextFlags = CONTEXT_CONTROL;
    if (!GetThreadContext(handle, &context))
    {
        CloseHandle(handle);
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }
#   if defined(ZYAN_X64)
    context.Rip = (DWORD64)ip;
#   else
    context.Eip = (DWORD)ip;
#   endif
    const BOOL ok = SetThreadContext(handle, &context);
    CloseHandle(handle);
    return ok ? ZYAN_STATUS_SUCCESS : ZYAN_STATUS_BAD_SYSTEMCALL;
#else
    // Register access is only implemented for x86-64 (and x86) this round.
    ZYAN_UNUSED(thread_id);
    ZYAN_UNUSED(ip);
    return ZYAN_STATUS_BAD_SYSTEMCALL;
#endif
}

#elif defined(ZYAN_LINUX)

// The suspend signal parks the target thread inside its handler until it is resumed, exposing and
// optionally rewriting the thread's instruction pointer. A dedicated realtime signal is used so it
// does not collide with application signal handlers. Suspends are serialised by the caller.
#define ZYAN_THREAD_CONTROL_SIGNAL (SIGRTMIN + 6)

typedef struct ZyanThreadParkSlot_
{
    volatile pid_t        tid;
    volatile int          active;
    sem_t                 parked;   // handler -> controller: "I am parked, saved_ip is valid"
    sem_t                 resume;   // controller -> handler: "resume now"
    sem_t                 done;     // handler -> controller: "I applied new_ip and am returning"
    volatile ZyanUPointer saved_ip;
    volatile ZyanUPointer new_ip;
    volatile int          apply_ip;
} ZyanThreadParkSlot;

// Array of stable slot pointers. The pointer array may be reallocated, but individual slots never
// move, so a handler parked on its slot pointer is never invalidated.
//
// These globals are written by the controller thread (ZyanAcquireSlot) and read by the target
// thread's signal handler (ZyanFindSlot). `tgkill` entering the kernel and the subsequent signal
// delivery provide the memory barrier that publishes the controller's writes to the handler; that
// is the same ordering guarantee other signal-based thread-stop implementations rely on. Suspends
// are serialised by a single controller thread, so there are no concurrent writers. `volatile`
// additionally prevents the compiler from caching the loop bound, matching the per-slot fields.
static ZyanThreadParkSlot** g_slots = ZYAN_NULL;
static volatile ZyanUSize g_slot_count = 0;
static volatile ZyanUSize g_slot_capacity = 0;
static volatile int g_handler_installed = 0;

static pid_t ZyanGetTid(void)
{
    return (pid_t)syscall(SYS_gettid);
}

static ZyanThreadParkSlot* ZyanFindSlot(pid_t tid)
{
    for (ZyanUSize i = 0; i < g_slot_count; ++i)
    {
        if (g_slots[i]->active && (g_slots[i]->tid == tid))
        {
            return g_slots[i];
        }
    }
    return ZYAN_NULL;
}

static void ZyanThreadControlHandler(int sig, siginfo_t* info, void* ucontext)
{
    ZYAN_UNUSED(sig);
    ZYAN_UNUSED(info);

    ZyanThreadParkSlot* const slot = ZyanFindSlot(ZyanGetTid());
    if (!slot)
    {
        return; // spurious delivery for a thread we are not parking
    }

    ucontext_t* const uc = (ucontext_t*)ucontext;
    slot->saved_ip = (ZyanUPointer)uc->uc_mcontext.gregs[REG_RIP];
    sem_post(&slot->parked);

    while (sem_wait(&slot->resume) != 0)
    {
        // retry on EINTR
    }

    if (slot->apply_ip)
    {
        uc->uc_mcontext.gregs[REG_RIP] = (greg_t)slot->new_ip;
    }
    sem_post(&slot->done);
}

static ZyanStatus ZyanThreadControlEnsureHandler(void)
{
    if (g_handler_installed)
    {
        return ZYAN_STATUS_SUCCESS;
    }
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = &ZyanThreadControlHandler;
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sigfillset(&sa.sa_mask);
    if (sigaction(ZYAN_THREAD_CONTROL_SIGNAL, &sa, ZYAN_NULL) != 0)
    {
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }
    g_handler_installed = 1;
    return ZYAN_STATUS_SUCCESS;
}

static ZyanThreadParkSlot* ZyanAcquireSlot(void)
{
    for (ZyanUSize i = 0; i < g_slot_count; ++i)
    {
        if (!g_slots[i]->active)
        {
            return g_slots[i];
        }
    }
    if (g_slot_count == g_slot_capacity)
    {
        const ZyanUSize new_cap = g_slot_capacity ? (g_slot_capacity * 2) : 8;
        ZyanThreadParkSlot** const grown =
            (ZyanThreadParkSlot**)realloc(g_slots, new_cap * sizeof(ZyanThreadParkSlot*));
        if (!grown)
        {
            return ZYAN_NULL;
        }
        g_slots = grown;
        g_slot_capacity = new_cap;
    }
    ZyanThreadParkSlot* const slot = (ZyanThreadParkSlot*)malloc(sizeof(ZyanThreadParkSlot));
    if (!slot)
    {
        return ZYAN_NULL;
    }
    memset(slot, 0, sizeof(*slot));
    if ((sem_init(&slot->parked, 0, 0) != 0) ||
        (sem_init(&slot->resume, 0, 0) != 0) ||
        (sem_init(&slot->done, 0, 0) != 0))
    {
        free(slot);
        return ZYAN_NULL;
    }
    g_slots[g_slot_count++] = slot;
    return slot;
}

ZyanStatus ZyanThreadSuspend(ZyanThreadId thread_id)
{
    ZYAN_CHECK(ZyanThreadControlEnsureHandler());

    ZyanThreadParkSlot* const slot = ZyanAcquireSlot();
    if (!slot)
    {
        return ZYAN_STATUS_NOT_ENOUGH_MEMORY;
    }

    slot->tid      = (pid_t)thread_id;
    slot->saved_ip = 0;
    slot->new_ip   = 0;
    slot->apply_ip = 0;
    slot->active   = 1;               // publish before signalling
    __sync_synchronize();

    if (syscall(SYS_tgkill, getpid(), (pid_t)thread_id, ZYAN_THREAD_CONTROL_SIGNAL) != 0)
    {
        slot->active = 0;
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }

    while (sem_wait(&slot->parked) != 0)
    {
        // retry on EINTR
    }
    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanThreadResume(ZyanThreadId thread_id)
{
    ZyanThreadParkSlot* const slot = ZyanFindSlot((pid_t)thread_id);
    if (!slot)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

    sem_post(&slot->resume);
    while (sem_wait(&slot->done) != 0)
    {
        // retry on EINTR
    }
    slot->active = 0;                 // free the slot for reuse only after the handler returned
    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanThreadGetInstructionPointer(ZyanThreadId thread_id, ZyanUPointer* ip)
{
    if (!ip)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }
    ZyanThreadParkSlot* const slot = ZyanFindSlot((pid_t)thread_id);
    if (!slot)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }
    *ip = slot->saved_ip;
    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanThreadSetInstructionPointer(ZyanThreadId thread_id, ZyanUPointer ip)
{
    ZyanThreadParkSlot* const slot = ZyanFindSlot((pid_t)thread_id);
    if (!slot)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }
    slot->new_ip = ip;
    slot->apply_ip = 1;
    return ZYAN_STATUS_SUCCESS;
}

#elif defined(ZYAN_APPLE)

ZyanStatus ZyanThreadSuspend(ZyanThreadId thread_id)
{
    return (thread_suspend((thread_act_t)thread_id) == KERN_SUCCESS)
        ? ZYAN_STATUS_SUCCESS : ZYAN_STATUS_BAD_SYSTEMCALL;
}

ZyanStatus ZyanThreadResume(ZyanThreadId thread_id)
{
    return (thread_resume((thread_act_t)thread_id) == KERN_SUCCESS)
        ? ZYAN_STATUS_SUCCESS : ZYAN_STATUS_BAD_SYSTEMCALL;
}

ZyanStatus ZyanThreadGetInstructionPointer(ZyanThreadId thread_id, ZyanUPointer* ip)
{
    if (!ip)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }
#if defined(ZYAN_X64)
    x86_thread_state64_t state;
    mach_msg_type_number_t count = x86_THREAD_STATE64_COUNT;
    if (thread_get_state((thread_act_t)thread_id, x86_THREAD_STATE64,
        (thread_state_t)&state, &count) != KERN_SUCCESS)
    {
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }
    *ip = (ZyanUPointer)state.__rip;
    return ZYAN_STATUS_SUCCESS;
#else
    // Register access is only implemented for x86-64 this round.
    ZYAN_UNUSED(thread_id);
    ZYAN_UNUSED(ip);
    return ZYAN_STATUS_BAD_SYSTEMCALL;
#endif
}

ZyanStatus ZyanThreadSetInstructionPointer(ZyanThreadId thread_id, ZyanUPointer ip)
{
#if defined(ZYAN_X64)
    x86_thread_state64_t state;
    mach_msg_type_number_t count = x86_THREAD_STATE64_COUNT;
    if (thread_get_state((thread_act_t)thread_id, x86_THREAD_STATE64,
        (thread_state_t)&state, &count) != KERN_SUCCESS)
    {
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }
    state.__rip = (uint64_t)ip;
    if (thread_set_state((thread_act_t)thread_id, x86_THREAD_STATE64,
        (thread_state_t)&state, x86_THREAD_STATE64_COUNT) != KERN_SUCCESS)
    {
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }
    return ZYAN_STATUS_SUCCESS;
#else
    // Register access is only implemented for x86-64 this round.
    ZYAN_UNUSED(thread_id);
    ZYAN_UNUSED(ip);
    return ZYAN_STATUS_BAD_SYSTEMCALL;
#endif
}

#elif defined(ZYAN_POSIX)

ZyanStatus ZyanThreadSuspend(ZyanThreadId thread_id)
{
    ZYAN_UNUSED(thread_id);
    return ZYAN_STATUS_BAD_SYSTEMCALL;
}

ZyanStatus ZyanThreadResume(ZyanThreadId thread_id)
{
    ZYAN_UNUSED(thread_id);
    return ZYAN_STATUS_BAD_SYSTEMCALL;
}

ZyanStatus ZyanThreadGetInstructionPointer(ZyanThreadId thread_id, ZyanUPointer* ip)
{
    ZYAN_UNUSED(thread_id);
    ZYAN_UNUSED(ip);
    return ZYAN_STATUS_BAD_SYSTEMCALL;
}

ZyanStatus ZyanThreadSetInstructionPointer(ZyanThreadId thread_id, ZyanUPointer ip)
{
    ZYAN_UNUSED(thread_id);
    ZYAN_UNUSED(ip);
    return ZYAN_STATUS_BAD_SYSTEMCALL;
}

#endif

#endif /* ZYAN_NO_LIBC */
