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

#ifndef ZYCORE_API_MEMORY_H
#define ZYCORE_API_MEMORY_H

#include <Zycore/Defines.h>
#include <Zycore/Status.h>
#include <Zycore/Types.h>

#ifndef ZYAN_NO_LIBC

#ifdef __cplusplus
extern "C" {
#endif

#if   defined(ZYAN_WINDOWS)
#   include <windows.h>
#elif defined(ZYAN_POSIX)
#   include <sys/mman.h>
#else
#   error "Unsupported platform detected"
#endif

/* ============================================================================================== */
/* Enums and types                                                                                */
/* ============================================================================================== */

/**
 * Defines the `ZyanMemoryPageProtection` enum.
 */
typedef enum ZyanMemoryPageProtection_
{
#if   defined(ZYAN_WINDOWS)

    ZYAN_PAGE_READONLY          = PAGE_READONLY,
    ZYAN_PAGE_READWRITE         = PAGE_READWRITE,
    ZYAN_PAGE_EXECUTE           = PAGE_EXECUTE,
    ZYAN_PAGE_EXECUTE_READ      = PAGE_EXECUTE_READ,
    ZYAN_PAGE_EXECUTE_READWRITE = PAGE_EXECUTE_READWRITE

#elif defined(ZYAN_POSIX)

    ZYAN_PAGE_READONLY          = PROT_READ,
    ZYAN_PAGE_READWRITE         = PROT_READ | PROT_WRITE,
    ZYAN_PAGE_EXECUTE           = PROT_EXEC,
    ZYAN_PAGE_EXECUTE_READ      = PROT_EXEC | PROT_READ,
    ZYAN_PAGE_EXECUTE_READWRITE = PROT_EXEC | PROT_READ | PROT_WRITE

#endif
} ZyanMemoryPageProtection;

/**
 * Defines the `ZyanMemoryRegionState` enum.
 */
typedef enum ZyanMemoryRegionState_
{
    ZYAN_MEMORY_REGION_STATE_FREE,
    ZYAN_MEMORY_REGION_STATE_RESERVED,
    ZYAN_MEMORY_REGION_STATE_COMMITTED
} ZyanMemoryRegionState;

/**
 * Defines the `ZyanMemoryRegionInfo` struct returned by `ZyanMemoryVirtualQuery`.
 *
 * `protection` is only meaningful when `state` is `ZYAN_MEMORY_REGION_STATE_COMMITTED`.
 */
typedef struct ZyanMemoryRegionInfo_
{
    void* base;
    ZyanUSize size;
    ZyanMemoryRegionState state;
    ZyanMemoryPageProtection protection;
} ZyanMemoryRegionInfo;

/* ============================================================================================== */
/* Exported functions                                                                             */
/* ============================================================================================== */

/* ---------------------------------------------------------------------------------------------- */
/* General                                                                                        */
/* ---------------------------------------------------------------------------------------------- */

/**
 * Returns the system page size.
 *
 * @return  The system page size.
 */
ZYCORE_EXPORT ZyanU32 ZyanMemoryGetSystemPageSize(void);

/**
 * Returns the system allocation granularity.
 *
 * The system allocation granularity specifies the minimum amount of bytes which can be allocated
 * at a specific address by a single call of `ZyanMemoryVirtualAlloc`.
 *
 * This value is typically 64KiB on Windows systems and equal to the page size on most POSIX
 * platforms.
 *
 * @return  The system allocation granularity.
 */
ZYCORE_EXPORT ZyanU32 ZyanMemoryGetSystemAllocationGranularity(void);

/* ---------------------------------------------------------------------------------------------- */
/* Memory management                                                                              */
/* ---------------------------------------------------------------------------------------------- */

/**
 * Changes the memory protection value of one or more pages.
 *
 * @param   address     The start address aligned to a page boundary.
 * @param   size        The size.
 * @param   protection  The new page protection value.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanMemoryVirtualProtect(void* address, ZyanUSize size,
    ZyanMemoryPageProtection protection);

/**
 * Releases one or more memory pages starting at the given address.
 *
 * @param   address The start address aligned to a page boundary.
 * @param   size    The size.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanMemoryVirtualFree(void* address, ZyanUSize size);

/**
 * Reserves and commits `size` bytes of memory with the given `protection`.
 *
 * @param   address     On entry, the requested base (aligned to the system allocation
 *                      granularity) or `ZYAN_NULL` to let the system choose. On success,
 *                      receives the actual base.
 * @param   size        The number of bytes to allocate.
 * @param   protection  The initial page protection.
 *
 * A non-null requested base that cannot be satisfied exactly fails with
 * `ZYAN_STATUS_BAD_SYSTEMCALL` rather than allocating elsewhere.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanMemoryVirtualAlloc(void** address, ZyanUSize size,
    ZyanMemoryPageProtection protection);

/**
 * Returns information about the virtual-memory region that contains `address`.
 *
 * @param   address The address to query.
 * @param   info    Receives the region information.
 *
 * `base <= address < base + size` always holds. Free (unmapped) ranges are reported as
 * `ZYAN_MEMORY_REGION_STATE_FREE`; on POSIX they are synthesized from the gaps between
 * mappings. On macOS a free region may report `base == address` rather than the true
 * lower bound of the gap; callers must rely only on `state`, `size`, and the invariant
 * above. The sole exception is `address` at the very top of the address space (the last
 * representable byte), which is never mappable in practice.
 *
 * @return  A zyan status code.
 */
ZYCORE_EXPORT ZyanStatus ZyanMemoryVirtualQuery(const void* address,
    ZyanMemoryRegionInfo* info);

/* ---------------------------------------------------------------------------------------------- */

/* ============================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* ZYAN_NO_LIBC */

#endif /* ZYCORE_API_MEMORY_H */
