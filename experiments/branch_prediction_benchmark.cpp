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
/*
Assembly code for mod2 on ARM (macbook):

    stp    x29, x30, [sp, #-0x20]!
    mov    x29, sp
    str    wzr, [x29, #0x1c]         // Stores 0 in int x
    str    wzr, [x29, #0x18]         // Stores 0 in int i
    b      0x104805254               // branch jump to (ldr w1, [x29, #0x18]) command
    ldr    w0, [x29, #0x18]
    and    w0, w0, #0x1             // Checks if i is even, there is no modulo operation
    cmp    w0, #0x0                 // Sets NZCV flags, which the below command then uses to determine if b.ne
    b.ne   0x104805248              // Jump to (ldr w0, [x29, #0x18]) command if Z flag is 0, which is true if cmp command evaluates to 0
    ldr    w0, [x29, #0x1c]         // This and next 2 commands evaluate to ++x
    add    w0, w0, #0x1
    str    w0, [x29, #0x1c]
    ldr    w0, [x29, #0x1c]         // This and next 2 commands evaluate to ++x
    add    w0, w0, #0x1
    str    w0, [x29, #0x1c]
    ldr    w0, [x29, #0x18]         // This and next 2 commands evaluate to ++i
    add    w0, w0, #0x1
    str    w0, [x29, #0x18]
    ldr    w1, [x29, #0x18]         // This and next 2 commands evaluate to comparing i to 10000
    mov    w0, #0x270f
    cmp    w1, w0
    b.le   0x104805220             // Jump to (ldr w0, [x29, #0x18]) command above if
    ldr    w0, [x29, #0x1c]
    ldp    x29, x30, [sp], #0x20
    ret

 */


// TODO: Analyze assembly for mod3 function
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
// The above are a bit flawed, because of

/*
    int count(const std::vector<int>& values)
    {
        int x = 0;

        for (int value : values) {
            if (value != 0) {
                ++x;
            }
        }

        return x;
    }
 */



BENCHMARK(BM_MajorityMiss);
BENCHMARK(BM_MajorityHit);