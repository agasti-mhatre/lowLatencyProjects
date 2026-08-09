project(MyBenchmark)

include(FetchContent)

FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.9.4
)

set(BENCHMARK_ENABLE_TESTING OFF)

FetchContent_MakeAvailable(googlebenchmark)

add_executable(branch_prediction_benchmark branch_prediction_benchmark.cpp)

target_link_libraries(
        branch_prediction_benchmark
        benchmark::benchmark_main
)