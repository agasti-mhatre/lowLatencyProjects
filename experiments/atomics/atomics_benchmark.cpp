#include <benchmark/benchmark.h>

#include <thread>
#include <mutex>
#include <atomic>

// lldb ./cmake-build-release/experiments/atomics/atomics_benchmark
// disassemble --frame --mixed -> see line-by-line breakdown

// breakpoint set -n "mutex_increment"
int mutex_increment(int n)
{
    int x = 0; // image lookup -v -f atomics_benchmark.cpp -l 12
    std::mutex mut;
    auto inc = [&mut, &x](int n)
    {
        for (int i = 0; i < n; ++i)
        {
            std::lock_guard<std::mutex> lock_guard(mut);
            ++x;
        }
    };

    std::thread one{ inc, n };
    std::thread two{inc,  n };

    one.join();
    two.join();

    return x;
}

int atomic_increment(int n)
{
    std::atomic<int> x = 0;
    auto info = [&x](int n)
    {
        for (int i = 0; i < n; ++i)
        {
            ++x;
        }
    };

    std::thread one{info, n};
    std::thread two{info, n};

    one.join();
    two.join();

    return x;
}

static void BENCHMARK_MUTEX(benchmark::State& state)
{
    int n = 500;
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(n);
        auto result = mutex_increment(n);
        benchmark::DoNotOptimize(result);
    }
}

static void BENCHMARK_ATOMIC(benchmark::State& state)
{
    int n = 500;
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(n);
        auto result = atomic_increment(n);
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK(BENCHMARK_MUTEX);
BENCHMARK(BENCHMARK_ATOMIC);