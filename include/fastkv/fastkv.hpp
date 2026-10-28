#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace fastkv {

// Thread-safe in-memory key-value store.
//
// Keys are spread over a fixed number of shards by std::hash. Each shard owns
// its own std::shared_mutex and std::unordered_map, so readers never block each
// other and writers only block operations that land on the same shard.
class FastKV {
public:
    static constexpr std::size_t kDefaultShardCount = 16;

    // Throws std::invalid_argument if shard_count is zero.
    explicit FastKV(std::size_t shard_count = kDefaultShardCount);

    FastKV(const FastKV&) = delete;
    FastKV& operator=(const FastKV&) = delete;
    FastKV(FastKV&&) = delete;
    FastKV& operator=(FastKV&&) = delete;
    ~FastKV();

    void put(const std::string& key, const std::string& value);
    // Returns std::nullopt when the key is absent, so an absent key and a key
    // stored with an empty value are distinguishable.
    std::optional<std::string> get(const std::string& key) const;
    // Returns true if the key was present and has been removed.
    bool erase(const std::string& key);

    std::size_t shard_count() const noexcept { return shard_count_; }

private:
    struct Shard;

    Shard& shard_for(const std::string& key) const;

    std::size_t shard_count_;
    std::unique_ptr<Shard[]> shards_;
};

} // namespace fastkv
