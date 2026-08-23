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


// These next experiments all use optimized builds which require:
// 1. target_compile_options(branch_prediction_benchmark PRIVATE -O2)
// in the local CMakeLists.txt
// 2. __attribute__((noinline)) attribute above each function I want
// to put a breakpoint in

static const unsigned long k = 10000000;

__attribute__((noinline))
unsigned long modX(unsigned long x)
{
    unsigned long res = 0;
    for (unsigned long i = 0; i < k; ++i)
    {
        if ((i % x) == 0){ ++res; }
        else
        {
            for (unsigned long i = 0; i < (x - 1); ++i) { ++res; }
        }
    }
    return res;
}

// Do [[likely]]/[[unlikely]] experiments next
__attribute__((noinline))
unsigned long modXLikely(unsigned long x)
{
    unsigned long res = 0;
    for (unsigned long i = 0; i < k; ++i)
    {
        if ((i % x) == 0) [[likely]] { ++res; }
        else
        {
            for (unsigned long i = 0; i < (x - 1); ++i) { ++res; }
        }
    }
    return res;
}

// Including this __attribute__
// so that this function can be debugged
// at its breakpoints when optimized compilations
// are enabled.
__attribute__((noinline))
unsigned long modXUnlikely(unsigned long x)
{
    unsigned long res = 0;
    for (unsigned long i = 0; i < k; ++i)
    {
        if ((i % x) == 0) [[unlikely]] { ++res; }
        else
        {
            for (unsigned long i = 0; i < (x - 1); ++i) { ++res; }
        }
    }
    return res;
}

static void BM_ModX(benchmark::State& state) {
    unsigned long x = static_cast<unsigned long>(state.range(0));
    for (auto _ : state) {
        benchmark::DoNotOptimize(x);
        auto result = modX(x);
        benchmark::DoNotOptimize(result);
    }
}

static void BM_ModXLikely(benchmark::State& state) {
    unsigned long x = static_cast<unsigned long>(state.range(0));
    for (auto _ : state) {
        benchmark::DoNotOptimize(x);
        auto result = modX(x);
        benchmark::DoNotOptimize(result);
    }
}

static void BM_ModXUnlikely(benchmark::State& state) {
    unsigned long x = static_cast<unsigned long>(state.range(0));
    for (auto _ : state) {
        benchmark::DoNotOptimize(x);
        auto result = modX(x);
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK(BM_ModX)->Arg(100);
BENCHMARK(BM_ModX)->Arg(117);
BENCHMARK(BM_ModX)->Arg(500);
BENCHMARK(BM_ModX)->Arg(10000);
BENCHMARK(BM_ModX)->Arg(k);

BENCHMARK(BM_ModXLikely)->Arg(100);
BENCHMARK(BM_ModXLikely)->Arg(117);
BENCHMARK(BM_ModXLikely)->Arg(500);
BENCHMARK(BM_ModXLikely)->Arg(10000);
BENCHMARK(BM_ModXLikely)->Arg(k);

BENCHMARK(BM_ModXUnlikely)->Arg(100);
BENCHMARK(BM_ModXUnlikely)->Arg(117);
BENCHMARK(BM_ModXUnlikely)->Arg(500);
BENCHMARK(BM_ModXUnlikely)->Arg(10000);
BENCHMARK(BM_ModXUnlikely)->Arg(k);

/*
-------------------------------------------------------------------
Benchmark                         Time             CPU   Iterations
-------------------------------------------------------------------
BM_ModX/100                 5347786 ns      5344431 ns          130
BM_ModX/117                 5322313 ns      5312924 ns          131
BM_ModX/500                 5336963 ns      5330664 ns          131
BM_ModX/10000               5379748 ns      5363763 ns          131
BM_ModX/10000000            5195748 ns      5185037 ns          135
BM_ModXLikely/100           5336776 ns      5330623 ns          130
BM_ModXLikely/117           5815688 ns      5651667 ns          132
BM_ModXLikely/500           5305938 ns      5305031 ns          131
BM_ModXLikely/10000         5318774 ns      5318331 ns          133
BM_ModXLikely/10000000      5169148 ns      5168259 ns          135
BM_ModXUnlikely/100         5321463 ns      5320409 ns          132
BM_ModXUnlikely/117         5308007 ns      5307098 ns          132
BM_ModXUnlikely/500         5314462 ns      5313386 ns          132
BM_ModXUnlikely/10000       5306268 ns      5305458 ns          131
BM_ModXUnlikely/10000000    5167044 ns      5165926 ns          136
*/

// Analyze assembly for attribute code above

/*
modX:

*/

/*
modLikelyX:

*/


// TODO: Do hot path, with for(if (...)) and if(for(...))