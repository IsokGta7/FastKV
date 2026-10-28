#include "fastkv/fastkv.hpp"

#include <mutex>

namespace fastkv {

void FastKV::put(const std::string& key, const std::string& value) {
    std::unique_lock lock(mutex_);
    store_[key] = value;
}

std::optional<std::string> FastKV::get(const std::string& key) const {
    std::shared_lock lock(mutex_);
    auto it = store_.find(key);
    if (it == store_.end()) {
        return std::nullopt;
    }
    return it->second;
}

bool FastKV::erase(const std::string& key) {
    std::unique_lock lock(mutex_);
    return store_.erase(key) > 0;
}

} // namespace fastkv
