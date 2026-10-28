#include "fastkv/fastkv.hpp"

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

FastKV::Shard& FastKV::shard_for(const std::string& key) const {
    return shards_[std::hash<std::string>{}(key) % shard_count_];
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
