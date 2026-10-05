#include <benchmark/benchmark.h>

#include <thread>
#include <mutex>
#include <atomic>

#include <iostream>

// Use https://godbolt.org/ to analyze assembly

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
            ++x; // bl std::__atomic_base<int>::operator++()
        }
    };

    std::thread one{info, n};
    std::thread two{info, n};

    one.join();
    two.join();

    return x;
}

int cas_strong_increment(int n)
{
    std::atomic<int> x = 0;
    auto info = [&x](int n)
    {
        int i = 0;
        while (i < n)
        {
            int expected = x.load();
            i += x.compare_exchange_strong(expected, expected + 1);
        }
    };

    std::thread one{info, n};
    std::thread two{info, n};

    one.join();
    two.join();

    return x;
}

// TODO: Do spin lock comparison

constexpr int n = 500;

static void BENCHMARK_MUTEX(benchmark::State& state)
{
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(n);
        auto result = mutex_increment(n);
        benchmark::DoNotOptimize(result);
    }
}

static void BENCHMARK_ATOMIC(benchmark::State& state)
{
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(n);
        auto result = atomic_increment(n);
        benchmark::DoNotOptimize(result);
    }
}

static void BENCHMARK_CAS_STRONG(benchmark::State& state)
{
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(n);
        auto result = cas_strong_increment(n);
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK(BENCHMARK_MUTEX);
BENCHMARK(BENCHMARK_ATOMIC);
BENCHMARK(BENCHMARK_CAS_STRONG);

/*
---------------------------------------------------------------
Benchmark                     Time             CPU   Iterations
---------------------------------------------------------------
BENCHMARK_MUTEX           40352 ns        12069 ns        55942
BENCHMARK_ATOMIC          17842 ns        11761 ns        59728
BENCHMARK_CAS_STRONG      24071 ns        10994 ns        63465

CAS is slower because it is conditional, unlike the basic atomic example
where fetch_add is carried out unconditionally.

 */