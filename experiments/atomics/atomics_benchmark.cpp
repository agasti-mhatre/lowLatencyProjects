#include <benchmark/benchmark.h>

#include <thread>
#include <mutex>
#include <atomic>
#include <vector>

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

int cas_weak_increment(int n)
{
    std::atomic<int> x = 0;
    auto info = [&x](int n)
    {
        int i = 0;
        while (i < n)
        {
            int expected = x.load();
            i += x.compare_exchange_weak(expected, expected + 1);
        }
    };

    std::thread one{info, n};
    std::thread two{info, n};

    one.join();
    two.join();

    return x;
}

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

static void BENCHMARK_CAS_WEAK(benchmark::State& state)
{
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(n);
        auto result = cas_weak_increment(n);
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK(BENCHMARK_MUTEX);
BENCHMARK(BENCHMARK_ATOMIC);
BENCHMARK(BENCHMARK_CAS_STRONG);
BENCHMARK(BENCHMARK_CAS_WEAK);

/*
---------------------------------------------------------------
Benchmark                     Time             CPU   Iterations
---------------------------------------------------------------
BENCHMARK_MUTEX           39337 ns        12121 ns        57116
BENCHMARK_ATOMIC          16660 ns        11409 ns        57746
BENCHMARK_CAS_STRONG      23008 ns        10632 ns        65092
BENCHMARK_CAS_WEAK        22926 ns        10616 ns        65309

 - CAS is slower because it is conditional, unlike the basic atomic example
where fetch_add is carried out unconditionally.
 */

///  HIGH CONTENTION   ///

int mutex_increment_high_cont(int n)
{
    int x = 0;
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

int atomic_increment_high_cont(int n)
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

constexpr int n2 = 50;

static void BENCHMARK_MUTEX_HIGH_CONT(benchmark::State& state)
{
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(n);
        auto result = mutex_increment(n);
        benchmark::DoNotOptimize(result);
    }
}

static void BENCHMARK_ATOMIC_HIGH_CONT(benchmark::State& state)
{
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(n);
        auto result = atomic_increment(n);
        benchmark::DoNotOptimize(result);
    }
}


BENCHMARK(BENCHMARK_MUTEX_HIGH_CONT);
BENCHMARK(BENCHMARK_ATOMIC_HIGH_CONT);


// TODO: Do spin lock comparison
// TODO: Investigate CAS weak vs CAS strong memory ordering in godbolt