#include "fastkv/fastkv.hpp"

#include <cstdint>
#include <functional>
#include <mutex>
#include <stdexcept>

namespace fastkv {

// Aligned to a typical cache line so that the locks of neighbouring shards do
// not share one and bounce between cores.
struct alignas(64) FastKV::Shard {
    mutable std::shared_mutex mutex;
    std::unordered_map<std::string, std::string> map;
};

FastKV::FastKV(std::size_t shard_count) : shard_count_(shard_count) {
    if (shard_count == 0) {
        throw std::invalid_argument("fastkv: shard_count must be at least 1");
    }
    shards_ = std::make_unique<Shard[]>(shard_count);
}

FastKV::~FastKV() = default;

namespace {

// Each shard's unordered_map hashes the key again with the same std::hash.
// Standard libraries that size buckets as a power of two (MSVC's does) pick the
// bucket from the low bits of that hash, so choosing the shard from the same
// low bits would leave every shard using only 1/shard_count of its buckets.
// Mixing the hash first (the splitmix64 finaliser) makes the shard index
// independent of the bits the map uses.
std::uint64_t mix(std::uint64_t x) noexcept {
    x ^= x >> 30;
    x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27;
    x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return x;
}

} // namespace

FastKV::Shard& FastKV::shard_for(const std::string& key) const {
    const std::uint64_t h = std::hash<std::string>{}(key);
    return shards_[static_cast<std::size_t>(mix(h) % shard_count_)];
}

void FastKV::put(const std::string& key, const std::string& value) {
    Shard& shard = shard_for(key);
    std::unique_lock lock(shard.mutex);
    shard.map.insert_or_assign(key, value);
}

std::optional<std::string> FastKV::get(const std::string& key) const {
    const Shard& shard = shard_for(key);
    std::shared_lock lock(shard.mutex);
    auto it = shard.map.find(key);
    if (it == shard.map.end()) {
        return std::nullopt;
    }
    return it->second;
}

bool FastKV::erase(const std::string& key) {
    Shard& shard = shard_for(key);
    std::unique_lock lock(shard.mutex);
    return shard.map.erase(key) > 0;
}

} // namespace fastkv
