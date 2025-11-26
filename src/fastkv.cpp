#include "../include/fastkv/fastkv.hpp"

#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <string>


namespace fastkv {

void FastKV::put(const std::string& key, const std::string& value) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    store_[key] = value;
}

std::string FastKV::get(const std::string& key) const {
    std::shared_lock lock(mutex_);
    auto iterator = store_.find(key);
    return (iterator != store_.end()) ? iterator->second : "";
}

bool FastKV::erase(const std::string& key) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    return store_.erase(key) > 0;
}

} // namespace fastkv
