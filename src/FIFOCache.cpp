#include "FIFOCache.h"

FIFOCache::FIFOCache(int capacity)
    : capacity_(capacity)
    , hits_(0)
    , misses_(0)
{}

int FIFOCache::get(int key) {
    auto it = map_.find(key);
    if (it != map_.end()) {
        ++hits_;
        return it->second;
    }
    ++misses_;
    return -1;
}

void FIFOCache::put(int key, int value) {
    if (map_.count(key)) {
        map_[key] = value;
        return;
    }
    
    if (static_cast<int>(map_.size()) == capacity_) {
        while (!order_.empty() && map_.find(order_.front()) == map_.end()) {
            order_.pop();
        }
        if (!order_.empty()) {
            map_.erase(order_.front());
            order_.pop();
        }
    }
    
    map_[key] = value;
    order_.push(key);
}

void FIFOCache::resetStats() {
    hits_ = 0;
    misses_ = 0;
}

void FIFOCache::clear() {
    map_.clear();
    while (!order_.empty()) order_.pop();
    resetStats();
}

int FIFOCache::size() const {
    return static_cast<int>(map_.size());
}
