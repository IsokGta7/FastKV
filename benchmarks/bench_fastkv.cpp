#include "fastkv/fastkv.hpp"
#include <benchmark/benchmark.h>

static void BM_Put(benchmark::State& state) {
    fastkv::FastKV kv;
    for (auto _ : state) {
        kv.put("key", "value");
    }
}
BENCHMARK(BM_Put);

BENCHMARK_MAIN();
