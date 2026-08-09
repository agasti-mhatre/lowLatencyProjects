#include <benchmark/benchmark.h>

// clang++ -std=c++23 -O3 -S experiments/branch_prediction_benchmark.cpp -o branch_prediction.s
// inspect compiled binary: otool -tvV cmake-build-debug/experiments/branch_prediction_benchmark

int noPrediction()
{
    int x = 0;
    for (int i = 0; i < 10000; ++i) { ++x; }
    return x;
}

int mod2()
{
    int x = 0;
    for (int i = 0; i < 10000; ++i)
    {
        if ((i % 2) == 0)
        {
            ++x;
            ++x;
        }
    }
    return x;
}

int mod3()
{
    int x = 0;
    for (int i = 0; i < 10000; ++i)
    {
        if ((i % 3) == 0)
        {
            ++x;
            ++x;
            ++x;
        }
    }
    return x;
}

int mod5()
{
    int x = 0;
    for (int i = 0; i < 10000; ++i)
    {
        if ((i % 5) == 0)
        {
            ++x;
            ++x;
            ++x;
            ++x;
            ++x;
        }
    }
    return x;
}

int mod1000()
{
    int x = 0;
    for (int i = 0; i < 10000; ++i)
    {
        if ((i % 1000) == 0)
        {
            for (int i = 0; i < 1000; ++i) {++x;}
        }
    }
    return x;
}

int majorityMiss()
{
    int x = 0;
    for (int i = 0; i < 10000; ++i)
    {
        if ((i == 5) || (i == 15) || (i == 100) || (i == 150) || (i == 300) || (i == 9035))
        {
            ++x;
        }
    }
    return x;
}

int majorityHit()
{
    int x = 0;
    for (int i = 0; i < 10000; ++i)
    {
        if ((i != 5) && (i != 15) && (i != 100) && (i != 150) && (i != 300) && (i != 9035))
        {
            ++x;
        }
    }
    return x;
}

// Do [[likely]] and [[unlikely]] experiments
// Maybe test random()?

static void BM_NoPrediction(benchmark::State& state)
{
    for (auto _ : state) {
        benchmark::DoNotOptimize(noPrediction());
    }
}

static void BM_Mod2(benchmark::State& state)
{
    for (auto _ : state) {
        benchmark::DoNotOptimize(mod2());
    }
}

static void BM_Mod3(benchmark::State& state)
{
    for (auto _ : state) {
        benchmark::DoNotOptimize(mod3());
    }
}

static void BM_Mod5(benchmark::State& state)
{
    for (auto _ : state) {
        benchmark::DoNotOptimize(mod5());
    }
}

static void BM_Mod1000(benchmark::State& state)
{
    for (auto _ : state) {
        benchmark::DoNotOptimize(mod1000());
    }
}

static void BM_MajorityMiss(benchmark::State& state)
{
    for (auto _ : state) {
        benchmark::DoNotOptimize(majorityMiss());
    }
}
static void BM_MajorityHit(benchmark::State& state)
{
    for (auto _ : state) {
        benchmark::DoNotOptimize(majorityHit());
    }
}

BENCHMARK(BM_NoPrediction);
BENCHMARK(BM_Mod2);
BENCHMARK(BM_Mod3);
BENCHMARK(BM_Mod5);
BENCHMARK(BM_Mod1000);
BENCHMARK(BM_MajorityMiss);
BENCHMARK(BM_MajorityHit);