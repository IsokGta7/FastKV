#include "fastkv/fastkv.hpp"

#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace {

constexpr std::size_t kKeyCount = 1 << 16;

const std::vector<std::string>& keys() {
    static const std::vector<std::string> k = [] {
        std::vector<std::string> v;
        v.reserve(kKeyCount);
        for (std::size_t i = 0; i < kKeyCount; ++i) {
            v.push_back("key-" + std::to_string(i));
        }
        return v;
    }();
    return k;
}

// Small, fast, per-thread PRNG so the generator itself is not a shared
// bottleneck.
struct XorShift64 {
    std::uint64_t state;
    std::uint64_t next() {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        return state;
    }
};

std::unique_ptr<fastkv::FastKV> g_store;

// Shared store, uniformly random keys, a fixed fraction of writes.
// Args: {shard count, write percentage}. Run with 1..N threads.
void BM_MixedWorkload(benchmark::State& state) {
    const auto shards = static_cast<std::size_t>(state.range(0));
    const auto write_percent = static_cast<std::uint64_t>(state.range(1));
    const auto& k = keys();

    if (state.thread_index() == 0) {
        g_store = std::make_unique<fastkv::FastKV>(shards);
        for (const auto& key : k) {
            g_store->put(key, "initial");
        }
    }

    XorShift64 rng{0x9E3779B97F4A7C15ULL + static_cast<std::uint64_t>(state.thread_index())};
    const std::string value = "value";
    for (auto _ : state) {
        const std::uint64_t r = rng.next();
        const std::string& key = k[r % kKeyCount];
        if ((r >> 32) % 100 < write_percent) {
            g_store->put(key, value);
        } else {
            benchmark::DoNotOptimize(g_store->get(key));
        }
    }
    state.SetItemsProcessed(state.iterations());

    if (state.thread_index() == 0) {
        g_store.reset();
    }
}

BENCHMARK(BM_MixedWorkload)
    ->ArgNames({"shards", "write%"})
    ->ArgsProduct({{1, 16}, {10, 50}})
    ->ThreadRange(1, 8)
    ->UseRealTime();

} // namespace

BENCHMARK_MAIN();
