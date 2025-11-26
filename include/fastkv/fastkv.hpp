#pragma once

#include <string>
#include <unordered_map>
#include <shared_mutex>

namespace fastkv {

class FastKV {
public:
    FastKV() = default;

    void put(const std::string& key, const std::string& value);
    std::string get(const std::string& key) const;
    bool erase(const std::string& key);

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, std::string> store_;
};

} // namespace fastkv
