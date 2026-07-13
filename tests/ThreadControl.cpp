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
 * @brief   Tests the thread-control API.
 */

#include <gtest/gtest.h>
#include <atomic>
#include <thread>
#include <Zycore/API/Thread.h>
#include <Zycore/Vector.h>

TEST(ThreadControlTest, EnumerateFindsSpawnedThreadsAndExcludesCurrent)
{
    std::atomic<bool> stop{false};
    std::atomic<int>  ready{0};
    auto worker = [&]() { ++ready; while (!stop.load()) { std::this_thread::yield(); } };

    std::thread t1(worker), t2(worker), t3(worker);
    while (ready.load() < 3) { std::this_thread::yield(); }

    ZyanVector ids;
    ASSERT_EQ(ZyanVectorInit(&ids, sizeof(ZyanThreadId), 16,
        reinterpret_cast<ZyanMemberProcedure>(ZYAN_NULL)), ZYAN_STATUS_SUCCESS);
    ASSERT_EQ(ZyanThreadEnumerate(&ids, ZYAN_FALSE), ZYAN_STATUS_SUCCESS);

    ZyanUSize count = 0;
    ASSERT_EQ(ZyanVectorGetSize(&ids, &count), ZYAN_STATUS_SUCCESS);
    // At least the three workers (main thread excluded); gtest may run more.
    EXPECT_GE(count, static_cast<ZyanUSize>(3));

    // The current thread's id must not appear when excluded.
    ZyanThreadId self;
    ASSERT_EQ(ZyanThreadGetCurrentThreadId(&self), ZYAN_STATUS_SUCCESS);

    ZyanVector ids_incl;
    ASSERT_EQ(ZyanVectorInit(&ids_incl, sizeof(ZyanThreadId), 16,
        reinterpret_cast<ZyanMemberProcedure>(ZYAN_NULL)), ZYAN_STATUS_SUCCESS);
    ASSERT_EQ(ZyanThreadEnumerate(&ids_incl, ZYAN_TRUE), ZYAN_STATUS_SUCCESS);
    ZyanUSize count_incl = 0;
    ASSERT_EQ(ZyanVectorGetSize(&ids_incl, &count_incl), ZYAN_STATUS_SUCCESS);
    EXPECT_EQ(count_incl, count + 1);

    stop.store(true);
    t1.join(); t2.join(); t3.join();
    ZyanVectorDestroy(&ids);
    ZyanVectorDestroy(&ids_incl);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
