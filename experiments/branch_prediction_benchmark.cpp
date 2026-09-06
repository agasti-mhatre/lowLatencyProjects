#include <benchmark/benchmark.h>

#include <vector>

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
// 1. RelWithDebInfo CMake profile
// 2. __attribute__((noinline)) attribute above each function I want
// to put a breakpoint in
// optional instead of doing #1:
// target_compile_options(branch_prediction_benchmark PRIVATE -O2)
// in the local CMakeLists.txt

static const unsigned long k = 1000000;

__attribute__((noinline))
unsigned long modX(unsigned long x)
{
    unsigned long res;
    for (unsigned long i = 0; i < k; ++i)
    {
        if ((i % x) == 0)
        {
            ++res;
        }
    }
    return res;
}
// How to use lldb without Clion Debugger:
// - lldb ./cmake-build-release/experiments/branch_prediction_benchmark
// - breakpoint set -n 'modX(unsigned long)'
// - run
// - disassemble -f

// Do [[likely]]/[[unlikely]] experiments next
__attribute__((noinline))
unsigned long modXLikely(unsigned long x)
{
    unsigned long res;
    for (unsigned long i = 0; i < k; ++i)
    {
        if ((i % x) == 0) [[likely]]
        {
            ++res;
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
    unsigned long res;
    for (unsigned long i = 0; i < k; ++i)
    {
        if ((i % x) == 0) [[unlikely]]
        {
            ++res;
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
        auto result = modXLikely(x);
        benchmark::DoNotOptimize(result);
    }
}

static void BM_ModXUnlikely(benchmark::State& state) {
    unsigned long x = static_cast<unsigned long>(state.range(0));
    for (auto _ : state) {
        benchmark::DoNotOptimize(x);
        auto result = modXUnlikely(x);
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK(BM_ModX)->Arg(100);
BENCHMARK(BM_ModX)->Arg(117);
BENCHMARK(BM_ModX)->Arg(500);
BENCHMARK(BM_ModX)->Arg(1000);

BENCHMARK(BM_ModXLikely)->Arg(100);
BENCHMARK(BM_ModXLikely)->Arg(117);
BENCHMARK(BM_ModXLikely)->Arg(500);
BENCHMARK(BM_ModXLikely)->Arg(1000);

BENCHMARK(BM_ModXUnlikely)->Arg(100);
BENCHMARK(BM_ModXUnlikely)->Arg(117);
BENCHMARK(BM_ModXUnlikely)->Arg(500);
BENCHMARK(BM_ModXUnlikely)->Arg(1000);


/*
Only get numbers from RUN:
---------------------------------------------------------------
Benchmark                     Time             CPU   Iterations
---------------------------------------------------------------
BM_ModX/100             1137384 ns      1135455 ns          618
BM_ModX/117             1122337 ns      1118431 ns          624
BM_ModX/500             1005125 ns      1002088 ns          704
BM_ModX/1000             982119 ns       980727 ns          707
BM_ModXLikely/100       1139456 ns      1135823 ns          615
BM_ModXLikely/117       1118923 ns      1117054 ns          625
BM_ModXLikely/500        996263 ns       996011 ns          704
BM_ModXLikely/1000       977421 ns       977244 ns          716
BM_ModXUnlikely/100     1135108 ns      1133161 ns          621
BM_ModXUnlikely/117     1112026 ns      1111968 ns          628
BM_ModXUnlikely/500      996871 ns       996598 ns          704
BM_ModXUnlikely/1000     979419 ns       979299 ns          716
*/

/*
modX:
0000000100001350        mov     x4, #0x4240             x4 = 16960
0000000100001354        mov     x1, #0x0                x1 = 0
0000000100001358        movk    x4, #0xf, lsl #16
000000010000135c        nop
0000000100001360        udiv    x2, x1, x0
0000000100001364        msub    x2, x2, x0, x1
0000000100001368        add     x1, x1, #0x1
000000010000136c        cbnz    x2, 0x100001374
0000000100001370        add     x3, x3, #0x1
0000000100001374        cmp     x1, x4
0000000100001378        b.ne    0x100001360
000000010000137c        mov     x0, x3
0000000100001380        ret

- modXLikely/modXUnlikely have the same assembly code
- Verdict: Attributes like [[likely]]/[[unlikely]] are compiler hints,
which means that they may/may not reorganize assembly code. Thus, the
latencies are unaffected.

*/