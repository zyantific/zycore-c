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
 * @brief   Tests the virtual-memory API.
 */

#include <gtest/gtest.h>
#include <Zycore/API/Memory.h>

/* ============================================================================================== */
/* Tests                                                                                          */
/* ============================================================================================== */

TEST(MemoryTest, VirtualAllocFreeRoundtrip)
{
    void* address = ZYAN_NULL;
    ASSERT_EQ(ZyanMemoryVirtualAlloc(&address, 0x1000, ZYAN_PAGE_READWRITE),
        ZYAN_STATUS_SUCCESS);
    ASSERT_NE(address, ZYAN_NULL);

    volatile ZyanU8* const bytes = static_cast<ZyanU8*>(address);
    bytes[0]        = 0x42;
    bytes[0x1000-1] = 0x37;
    EXPECT_EQ(bytes[0], 0x42);
    EXPECT_EQ(bytes[0x1000-1], 0x37);

    EXPECT_EQ(ZyanMemoryVirtualFree(address, 0x1000), ZYAN_STATUS_SUCCESS);
}

TEST(MemoryTest, VirtualQueryReportsCommittedRegion)
{
    void* address = ZYAN_NULL;
    ASSERT_EQ(ZyanMemoryVirtualAlloc(&address, 0x1000, ZYAN_PAGE_READWRITE),
        ZYAN_STATUS_SUCCESS);

    ZyanMemoryRegionInfo info;
    ASSERT_EQ(ZyanMemoryVirtualQuery(address, &info), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(info.state, ZYAN_MEMORY_REGION_STATE_COMMITTED);
    EXPECT_LE(reinterpret_cast<ZyanUPointer>(info.base),
              reinterpret_cast<ZyanUPointer>(address));
    EXPECT_GE(reinterpret_cast<ZyanUPointer>(info.base) + info.size,
              reinterpret_cast<ZyanUPointer>(address) + 0x1000);

    ZyanMemoryVirtualFree(address, 0x1000);
}

TEST(MemoryTest, VirtualQueryReportsFreeRegion)
{
    // Allocate then free to obtain an address that is very likely unmapped afterwards.
    void* address = ZYAN_NULL;
    ASSERT_EQ(ZyanMemoryVirtualAlloc(&address, 0x1000, ZYAN_PAGE_READWRITE),
        ZYAN_STATUS_SUCCESS);
    ASSERT_EQ(ZyanMemoryVirtualFree(address, 0x1000), ZYAN_STATUS_SUCCESS);

    ZyanMemoryRegionInfo info;
    ASSERT_EQ(ZyanMemoryVirtualQuery(address, &info), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(info.state, ZYAN_MEMORY_REGION_STATE_FREE);
    EXPECT_LE(reinterpret_cast<ZyanUPointer>(info.base),
              reinterpret_cast<ZyanUPointer>(address));
    EXPECT_GT(reinterpret_cast<ZyanUPointer>(info.base) + info.size,
              reinterpret_cast<ZyanUPointer>(address));
}

TEST(MemoryTest, VirtualAllocFixedFailsWhenOccupied)
{
    void* address = ZYAN_NULL;
    ASSERT_EQ(ZyanMemoryVirtualAlloc(&address, 0x1000, ZYAN_PAGE_READWRITE),
        ZYAN_STATUS_SUCCESS);

    void* occupied = address; // same, now-mapped base
    EXPECT_NE(ZyanMemoryVirtualAlloc(&occupied, 0x1000, ZYAN_PAGE_READWRITE),
        ZYAN_STATUS_SUCCESS);

    ZyanMemoryVirtualFree(address, 0x1000);
}

/* ============================================================================================== */
/* Entry point                                                                                    */
/* ============================================================================================== */

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

/* ============================================================================================== */
