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

#include <Zycore/API/Memory.h>

#ifndef ZYAN_NO_LIBC

#if   defined(ZYAN_WINDOWS)

#elif defined(ZYAN_POSIX)
#   include <unistd.h>
#   include <stdio.h>
#   if defined(__APPLE__)
#       include <mach/mach.h>
#       include <mach/mach_vm.h>
#   endif
#   if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#       define MAP_ANONYMOUS MAP_ANON
#   endif
#else
#   error "Unsupported platform detected"
#endif

/* ============================================================================================== */
/* Exported functions                                                                             */
/* ============================================================================================== */

/* ---------------------------------------------------------------------------------------------- */
/* General                                                                                        */
/* ---------------------------------------------------------------------------------------------- */

ZyanU32 ZyanMemoryGetSystemPageSize(void)
{
#if defined(ZYAN_WINDOWS)

    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);

    return system_info.dwPageSize;

#elif defined(ZYAN_POSIX)

    return sysconf(_SC_PAGE_SIZE);

#endif
}

ZyanU32 ZyanMemoryGetSystemAllocationGranularity(void)
{
#if defined(ZYAN_WINDOWS)

    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);

    return system_info.dwAllocationGranularity;

#elif defined(ZYAN_POSIX)

    return sysconf(_SC_PAGE_SIZE);

#endif
}

/* ---------------------------------------------------------------------------------------------- */
/* Memory management                                                                              */
/* ---------------------------------------------------------------------------------------------- */

ZyanStatus ZyanMemoryVirtualProtect(void* address, ZyanUSize size, 
    ZyanMemoryPageProtection protection)
{
#if defined(ZYAN_WINDOWS)

    DWORD old;
    if (!VirtualProtect(address, size, protection, &old))
    {
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }

#elif defined(ZYAN_POSIX)

    if (mprotect(address, size, protection))
    {
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }

#endif

    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanMemoryVirtualFree(void* address, ZyanUSize size)
{
#if defined(ZYAN_WINDOWS)

    ZYAN_UNUSED(size);
    if (!VirtualFree(address, 0, MEM_RELEASE))
    {
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }

#elif defined(ZYAN_POSIX)

    if (munmap(address, size))
    {
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }

#endif

    return ZYAN_STATUS_SUCCESS;    
}

ZyanStatus ZyanMemoryVirtualAlloc(void** address, ZyanUSize size,
    ZyanMemoryPageProtection protection)
{
    if (!address || (size == 0))
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

#if defined(ZYAN_WINDOWS)

    void* const result = VirtualAlloc(*address, size, MEM_RESERVE | MEM_COMMIT, protection);
    if (!result)
    {
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }
    *address = result;

#elif defined(ZYAN_POSIX)

    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#if defined(MAP_FIXED_NOREPLACE)
    if (*address)
    {
        flags |= MAP_FIXED_NOREPLACE;
    }
#endif

    void* const result = mmap(*address, size, protection, flags, -1, 0);
    if (result == MAP_FAILED)
    {
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }

    // Reject rather than silently relocating when an exact base was requested but the
    // kernel ignored the hint (e.g. a platform without `MAP_FIXED_NOREPLACE`).
    if (*address && (result != *address))
    {
        munmap(result, size);
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }
    *address = result;

#endif

    return ZYAN_STATUS_SUCCESS;
}

ZyanStatus ZyanMemoryVirtualQuery(const void* address, ZyanMemoryRegionInfo* info)
{
    if (!info)
    {
        return ZYAN_STATUS_INVALID_ARGUMENT;
    }

#if defined(ZYAN_WINDOWS)

    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(address, &mbi, sizeof(mbi)) == 0)
    {
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }
    info->base = mbi.BaseAddress;
    info->size = mbi.RegionSize;
    switch (mbi.State)
    {
    case MEM_FREE:    info->state = ZYAN_MEMORY_REGION_STATE_FREE;      break;
    case MEM_RESERVE: info->state = ZYAN_MEMORY_REGION_STATE_RESERVED;  break;
    case MEM_COMMIT:  info->state = ZYAN_MEMORY_REGION_STATE_COMMITTED; break;
    default:          info->state = ZYAN_MEMORY_REGION_STATE_FREE;      break;
    }
    info->protection = (ZyanMemoryPageProtection)mbi.Protect;
    return ZYAN_STATUS_SUCCESS;

#elif defined(__linux__)

    const ZyanUPointer target = (ZyanUPointer)address;
    FILE* const maps = fopen("/proc/self/maps", "r");
    if (!maps)
    {
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }

    char line[512];
    ZyanUPointer prev_end = 0;
    ZyanBool found = ZYAN_FALSE;
    while (fgets(line, sizeof(line), maps))
    {
        unsigned long start, end;
        char perms[5] = { 0 };
        if (sscanf(line, "%lx-%lx %4s", &start, &end, perms) != 3)
        {
            continue;
        }
        if (target < start)
        {
            info->base       = (void*)prev_end;
            info->size       = (ZyanUSize)(start - prev_end);
            info->state      = ZYAN_MEMORY_REGION_STATE_FREE;
            info->protection = (ZyanMemoryPageProtection)0;
            found = ZYAN_TRUE;
            break;
        }
        if (target < end)
        {
            int prot = 0;
            if (perms[0] == 'r') prot |= PROT_READ;
            if (perms[1] == 'w') prot |= PROT_WRITE;
            if (perms[2] == 'x') prot |= PROT_EXEC;
            info->base       = (void*)start;
            info->size       = (ZyanUSize)(end - start);
            info->state      = ZYAN_MEMORY_REGION_STATE_COMMITTED;
            info->protection = (ZyanMemoryPageProtection)prot;
            found = ZYAN_TRUE;
            break;
        }
        prev_end = end;
    }
    fclose(maps);

    if (!found)
    {
        // Above all mappings: free up to the top of the address space.
        info->base       = (void*)prev_end;
        info->size       = (ZyanUSize)(~(ZyanUPointer)0 - prev_end);
        info->state      = ZYAN_MEMORY_REGION_STATE_FREE;
        info->protection = (ZyanMemoryPageProtection)0;
    }
    return ZYAN_STATUS_SUCCESS;

#elif defined(__APPLE__)

    mach_vm_address_t region_addr = (mach_vm_address_t)(ZyanUPointer)address;
    mach_vm_size_t region_size = 0;
    vm_region_basic_info_data_64_t vinfo;
    mach_msg_type_number_t count = VM_REGION_BASIC_INFO_COUNT_64;
    mach_port_t object_name = MACH_PORT_NULL;

    const kern_return_t kr = mach_vm_region(mach_task_self(), &region_addr, &region_size,
        VM_REGION_BASIC_INFO_64, (vm_region_info_t)&vinfo, &count, &object_name);
    if (kr == KERN_INVALID_ADDRESS)
    {
        info->base       = (void*)address;
        info->size       = (ZyanUSize)(~(ZyanUPointer)0 - (ZyanUPointer)address);
        info->state      = ZYAN_MEMORY_REGION_STATE_FREE;
        info->protection = (ZyanMemoryPageProtection)0;
        return ZYAN_STATUS_SUCCESS;
    }
    if (kr != KERN_SUCCESS)
    {
        return ZYAN_STATUS_BAD_SYSTEMCALL;
    }

    // `mach_vm_region` returns the region at or above the queried address. If it starts
    // above `address`, `address` sits in a free gap below it.
    if ((ZyanUPointer)region_addr > (ZyanUPointer)address)
    {
        info->base       = (void*)address;
        info->size       = (ZyanUSize)((ZyanUPointer)region_addr - (ZyanUPointer)address);
        info->state      = ZYAN_MEMORY_REGION_STATE_FREE;
        info->protection = (ZyanMemoryPageProtection)0;
        // Release the send-right returned by mach_vm_region to avoid leaking a Mach port.
        mach_port_deallocate(mach_task_self(), object_name);
        return ZYAN_STATUS_SUCCESS;
    }

    int prot = 0;
    if (vinfo.protection & VM_PROT_READ)    prot |= PROT_READ;
    if (vinfo.protection & VM_PROT_WRITE)   prot |= PROT_WRITE;
    if (vinfo.protection & VM_PROT_EXECUTE) prot |= PROT_EXEC;
    info->base       = (void*)(ZyanUPointer)region_addr;
    info->size       = (ZyanUSize)region_size;
    info->state      = ZYAN_MEMORY_REGION_STATE_COMMITTED;
    info->protection = (ZyanMemoryPageProtection)prot;
    mach_port_deallocate(mach_task_self(), object_name);
    return ZYAN_STATUS_SUCCESS;

#else
#   error "Unsupported platform detected"
#endif
}

/* ---------------------------------------------------------------------------------------------- */

/* ============================================================================================== */

#endif /* ZYAN_NO_LIBC */
