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

TEST(ThreadControlTest, SuspendStopsProgressResumeContinues)
{
    std::atomic<bool> stop{false};
    std::atomic<unsigned long> counter{0};
    std::atomic<ZyanThreadId> worker_id{0};

    std::thread worker([&]() {
        ZyanThreadId self = 0;
        // Publish this thread's enumeration id by finding the one that is not any other.
        // Simpler: the worker just runs; the test finds the id by enumeration below.
        ZYAN_UNUSED(self);
        while (!stop.load()) { counter.fetch_add(1, std::memory_order_relaxed); }
    });

    // Find the worker's id: enumerate excluding current; there should be exactly one extra
    // thread that keeps incrementing. We pick the id whose thread is making progress by
    // suspending each candidate and checking the counter; but to keep it deterministic we
    // run this test with only the worker as an extra thread.
    ZyanVector ids;
    ASSERT_EQ(ZyanVectorInit(&ids, sizeof(ZyanThreadId), 16,
        reinterpret_cast<ZyanMemberProcedure>(ZYAN_NULL)), ZYAN_STATUS_SUCCESS);
    // Give the worker a moment to start.
    while (counter.load() < 1000) { std::this_thread::yield(); }
    ASSERT_EQ(ZyanThreadEnumerate(&ids, ZYAN_FALSE), ZYAN_STATUS_SUCCESS);

    // Suspend every enumerated thread (gtest has no other busy threads here), then verify the
    // counter freezes.
    ZyanUSize n = 0; ZyanVectorGetSize(&ids, &n);
    for (ZyanUSize i = 0; i < n; ++i)
    {
        const ZyanThreadId id = *reinterpret_cast<const ZyanThreadId*>(ZyanVectorGet(&ids, i));
        ASSERT_EQ(ZyanThreadSuspend(id), ZYAN_STATUS_SUCCESS);
    }

    const unsigned long a = counter.load();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    const unsigned long b = counter.load();
    EXPECT_EQ(a, b); // frozen while suspended

    for (ZyanUSize i = 0; i < n; ++i)
    {
        const ZyanThreadId id = *reinterpret_cast<const ZyanThreadId*>(ZyanVectorGet(&ids, i));
        ASSERT_EQ(ZyanThreadResume(id), ZYAN_STATUS_SUCCESS);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_GT(counter.load(), b); // progressing again after resume

    stop.store(true);
    worker.join();
    ZyanVectorDestroy(&ids);
    ZYAN_UNUSED(worker_id);
}

// Exposes the address just past its spin loop so the test can redirect the instruction pointer
// there. `g_after_addr` is published once the thread is spinning.
//
// Only used by the labels-as-values based spin target below, so the whole block is guarded to
// avoid unused-variable warnings on compilers that fall into the GTEST_SKIP path.
#if defined(ZYAN_GNUC)
// g_ip_stop and g_ip_reached_end are intentionally `volatile bool`, not std::atomic: the spin
// loop's condition must compile to an inline memory read with NO function call, so that whenever
// the worker is suspended its instruction pointer is inside IpSpinTarget's own stack frame. That
// is what makes redirecting the IP to `after_loop` (a same-frame jump) stack-safe. An atomic load
// can compile to a real call, and suspending the worker inside that call would corrupt its stack
// when the IP is redirected away.
static volatile bool     g_ip_stop = false;
static volatile bool     g_ip_reached_end = false;
static void*             g_after_addr = nullptr;
static std::atomic<bool> g_ip_running{false};

static void IpSpinTarget()
{
    g_after_addr = &&after_loop;      // GCC/Clang labels-as-values extension
    g_ip_running.store(true);
loop:
    if (!g_ip_stop) { goto loop; }
after_loop:
    g_ip_reached_end = true;
}
#endif

TEST(ThreadControlTest, SetInstructionPointerRedirectsExecution)
{
#if !defined(ZYAN_GNUC)
    GTEST_SKIP() << "requires the computed-goto (labels-as-values) extension";
#else
    g_ip_stop = false;
    g_ip_reached_end = false;
    g_after_addr = nullptr;
    g_ip_running.store(false);

    std::thread worker(IpSpinTarget);
    while (!g_ip_running.load() || (g_after_addr == nullptr)) { std::this_thread::yield(); }

    ZyanVector ids;
    ASSERT_EQ(ZyanVectorInit(&ids, sizeof(ZyanThreadId), 16,
        reinterpret_cast<ZyanMemberProcedure>(ZYAN_NULL)), ZYAN_STATUS_SUCCESS);
    ASSERT_EQ(ZyanThreadEnumerate(&ids, ZYAN_FALSE), ZYAN_STATUS_SUCCESS);
    ZyanUSize n = 0; ZyanVectorGetSize(&ids, &n);
    ASSERT_GE(n, static_cast<ZyanUSize>(1));

    const ZyanThreadId id = *reinterpret_cast<const ZyanThreadId*>(ZyanVectorGet(&ids, 0));
    ASSERT_EQ(ZyanThreadSuspend(id), ZYAN_STATUS_SUCCESS);

    ZyanUPointer ip = 0;
    ASSERT_EQ(ZyanThreadGetInstructionPointer(id, &ip), ZYAN_STATUS_SUCCESS);
    EXPECT_NE(ip, static_cast<ZyanUPointer>(0));

    ASSERT_EQ(ZyanThreadSetInstructionPointer(id, reinterpret_cast<ZyanUPointer>(g_after_addr)),
        ZYAN_STATUS_SUCCESS);
    ASSERT_EQ(ZyanThreadResume(id), ZYAN_STATUS_SUCCESS);

    for (int i = 0; (i < 1000) && !g_ip_reached_end; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    EXPECT_TRUE(g_ip_reached_end);

    g_ip_stop = true; // safety net if the redirect did not take
    worker.join();
    ZyanVectorDestroy(&ids);
#endif
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
