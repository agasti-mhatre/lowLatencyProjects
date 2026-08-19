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
    mov    x29, sp                   // Move stack pointer into x29 (general purpose register)
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
    mov    w0, #0x270f              // Move 10,000 into w0
    cmp    w1, w0
    b.le   0x104805220             // Jump to (ldr w0, [x29, #0x18]) command above, if w1 (i) < w0 (10000)
    ldr    w0, [x29, #0x1c]
    ldp    x29, x30, [sp], #0x20  // x30 is the register for the return value
    ret
*/

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
/*
* mod3():
    stp    x29, x30, [sp, #-0x20]!
    mov    x29, sp                  // Set x29 as the stack pointer
    str    wzr, [x29, #0x1c]        // Set x = 0
    str    wzr, [x29, #0x18]        // Set i = 0
    b      0x1025852e8              // Jump to second (ldr w1, [x29, #0x18]) command
    ldr    w1, [x29, #0x18]         // Load i into w1 -> START Of MODULO Operation
    mov    w0, #0x5556              // Store 21846 into w0
    movk   w0, #0x5555, lsl #16     // Sets upper bits of w0. Goes from 0x00005556 -> 0x55555556
    smull  x0, w1, w0               // Multiply w0 and w1, store in x0
    lsr    x2, x0, #32              // Logical Right Shift: x2 = x0 >> 32
    asr    w0, w1, #31              // Arithmetic Right Shift: w0 = w1 >> 31
    sub    w2, w2, w0               // w2 -= w0
    mov    w0, w2                   // w0 = w2
    lsl    w0, w0, #1               // Logical Left Shift: w0 = 1 << w0
    add    w0, w0, w2               // w0 += w2
    sub    w2, w1, w0               // w2 = w1 - w0
    cmp    w2, #0x0                 // -> END OF MODULO Operation
    b.ne   0x1025852dc              // jump to (ldr w0, [x29, #0x18]) command if w2 is not equal to 0
    ldr    w0, [x29, #0x1c]         // load int x into w0
    add    w0, w0, #0x1             // ++x
    str    w0, [x29, #0x1c]         // write x back to memory
    ldr    w0, [x29, #0x1c]
    add    w0, w0, #0x1
    str    w0, [x29, #0x1c]
    ldr    w0, [x29, #0x1c]
    add    w0, w0, #0x1
    str    w0, [x29, #0x1c]
    ldr    w0, [x29, #0x18]         // load i into w0
    add    w0, w0, #0x1             // ++i
    str    w0, [x29, #0x18]         // write i back to memory
    ldr    w1, [x29, #0x18]         // Load i into w1
    mov    w0, #0x270f              // Load 10,000 into w0
    cmp    w1, w0                   // compare w1 (i) and w0 (10,000)
    b.le   0x102585284              // if w1 < w0, jump to first (ldr w1, [x29, #0x18]) command
    ldr    w0, [x29, #0x1c]
    ldp    x29, x30, [sp], #0x20
    ret

 *
 *
 *
 */

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

//BENCHMARK(BM_NoPrediction);
//BENCHMARK(BM_Mod2);
//BENCHMARK(BM_Mod3);
//BENCHMARK(BM_Mod5);
//BENCHMARK(BM_Mod1000);
//BENCHMARK(BM_MajorityMiss);
//BENCHMARK(BM_MajorityHit);
// The above are a bit flawed, because of uneven work
// that happens when a branch hit does happen. Load/add/store operations
// increase during a branch hit. Make the work even.

int modX(int x)
{
    int res = 0;
    for (int i = 0; i < 10000; ++i)
    {
        if ((i % x) == 0){ ++res; }
        else
        {
            for (int i = 0; i < (x - 1); ++i) { ++res; }
        }
    }
    return res;
}


static void BM_ModXEq1(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(modX(1));
    }
}

static void BM_ModXEq2(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(modX(2));
    }
}

static void BM_ModXEq3(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(modX(3));
    }
}

static void BM_ModXEq5(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(modX(5));
    }
}

static void BM_ModXEq10(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(modX(10));
    }
}

static void BM_ModXEq100(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(modX(100));
    }
}

static void BM_ModXEq1000(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(modX(1000));
    }
}

BENCHMARK(BM_ModXEq1);
BENCHMARK(BM_ModXEq2);
BENCHMARK(BM_ModXEq3);
BENCHMARK(BM_ModXEq5);
BENCHMARK(BM_ModXEq10);
BENCHMARK(BM_ModXEq100);
BENCHMARK(BM_ModXEq1000);