#pragma once

#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace fastkv {

class FastKV {
public:
    FastKV() = default;

    void put(const std::string& key, const std::string& value);
    // Returns std::nullopt when the key is absent, so an absent key and a key
    // stored with an empty value are distinguishable.
    std::optional<std::string> get(const std::string& key) const;
    // Returns true if the key was present and has been removed.
    bool erase(const std::string& key);

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, std::string> store_;
};

} // namespace fastkv
