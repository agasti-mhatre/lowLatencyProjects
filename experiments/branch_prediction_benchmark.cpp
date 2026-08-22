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

static void BM_ModX(benchmark::State& state) {
    int x = static_cast<int>(state.range(0));
    for (auto _ : state) {
        benchmark::DoNotOptimize(modX(x));
    }
}

BENCHMARK(BM_ModX)->Arg(1);
BENCHMARK(BM_ModX)->Arg(2);

/*
Address translations:

    - x29, #0x1c -> int x
    - x29, #0x2c -> int res
    - x29, #0x28 -> int i


    stp    x29, x30, [sp, #-0x30]!
    mov    x29, sp
    str    w0, [x29, #0x1c]          w0 = x command that was passed in. Store w0 into the address on right-hand side
    str    wzr, [x29, #0x2c]
    str    wzr, [x29, #0x28]
    b      0x104669e60               jump to (ldr w1, [x29, #0x28]) command
    ldr    w0, [x29, #0x28]          w0 = int i
    ldr    w1, [x29, #0x1c]          w1 = int x
    sdiv   w2, w0, w1                w2 = w0 / w1 (int i / int x)
    ldr    w1, [x29, #0x1c]          w1 = int x
    mul    w1, w2, w1                w1 = w2 * w1
    sub    w0, w0, w1                w0 = w0 - w1
    cmp    w0, #0x0                  is w0 == 0? ->
    b.ne   0x104669e20
    ldr    w0, [x29, #0x2c]
    add    w0, w0, #0x1
    str    w0, [x29, #0x2c]
    b      0x104669e54
    str    wzr, [x29, #0x24]
    b      0x104669e40
    ldr    w0, [x29, #0x2c]
    add    w0, w0, #0x1
    str    w0, [x29, #0x2c]
    ldr    w0, [x29, #0x24]
    add    w0, w0, #0x1
    str    w0, [x29, #0x24]
    ldr    w0, [x29, #0x1c]
    sub    w0, w0, #0x1
    ldr    w1, [x29, #0x24]
    cmp    w1, w0
    b.lt   0x104669e28
    ldr    w0, [x29, #0x28]
    add    w0, w0, #0x1
    str    w0, [x29, #0x28]
    ldr    w1, [x29, #0x28]           Load int i into register w1
    mov    w0, #0x270f                w0 = 9999
    cmp    w1, w0
    b.le   0x104669df0                if i <= 9999, jump to first (ldr w0, [x29, #0x28]) command
    ldr    w0, [x29, #0x2c]
    ldp    x29, x30, [sp], #0x30
    ret
 */

BENCHMARK(BM_ModX)->Arg(3);
BENCHMARK(BM_ModX)->Arg(5);
BENCHMARK(BM_ModX)->Arg(10);
BENCHMARK(BM_ModX)->Arg(100);

// Do [[likely]]/[[unlikely]] experiments next
int modXLikely(int x)
{
    int res = 0;
    for (int i = 0; i < 10000; ++i)
    {
        if ((i % x) == 0) [[likely]] { ++res; }
        else
        {
            for (int i = 0; i < (x - 1); ++i) { ++res; }
        }
    }
    return res;
}

int modXUnlikely(int x)
{
    int res = 0;
    for (int i = 0; i < 10000; ++i)
    {
        if ((i % x) == 0) [[unlikely]] { ++res; }
        else
        {
            for (int i = 0; i < (x - 1); ++i) { ++res; }
        }
    }
    return res;
}

static void BM_ModXLikely(benchmark::State& state) {
    int x = static_cast<int>(state.range(0));
    for (auto _ : state) {
        benchmark::DoNotOptimize(modXLikely(x));
    }
}

static void BM_ModXUnlikely(benchmark::State& state) {
    int x = static_cast<int>(state.range(0));
    for (auto _ : state) {
        benchmark::DoNotOptimize(modXUnlikely(x));
    }
}

BENCHMARK(BM_ModXLikely)->Arg(1);
BENCHMARK(BM_ModXLikely)->Arg(3);
BENCHMARK(BM_ModXLikely)->Arg(5);
BENCHMARK(BM_ModXLikely)->Arg(10);
BENCHMARK(BM_ModXLikely)->Arg(100);


BENCHMARK(BM_ModXUnlikely)->Arg(1);
BENCHMARK(BM_ModXUnlikely)->Arg(3);
BENCHMARK(BM_ModXUnlikely)->Arg(5);
BENCHMARK(BM_ModXUnlikely)->Arg(10);
BENCHMARK(BM_ModXUnlikely)->Arg(100);

/*
--------------------------------------------------------------
Benchmark                    Time             CPU   Iterations
--------------------------------------------------------------
BM_ModX/1                36595 ns        36591 ns        49123
BM_ModX/2                33383 ns        33381 ns        19290
BM_ModX/3                41815 ns        41814 ns        16639
BM_ModX/5                49206 ns        49201 ns        14446
BM_ModX/10               83715 ns        83706 ns         8362
BM_ModX/100            2187797 ns      2186464 ns          390
BM_ModXLikely/1          33894 ns        33889 ns        56034
BM_ModXLikely/3          38147 ns        38145 ns        16647
BM_ModXLikely/5          53549 ns        53523 ns        13888
BM_ModXLikely/10         83445 ns        83424 ns         8420
BM_ModXLikely/100      3054723 ns      3054718 ns          234
BM_ModXUnlikely/1        17145 ns        17121 ns        35411
BM_ModXUnlikely/3        39791 ns        39789 ns        16854
BM_ModXUnlikely/5        54430 ns        54411 ns        12929
BM_ModXUnlikely/10      113485 ns       113435 ns         6366
BM_ModXUnlikely/100    2218717 ns      2218730 ns          293

Explanation:
*/



// TODO: Do hot path, with for(if (...)) and if(for(...))