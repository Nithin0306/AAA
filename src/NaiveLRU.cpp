#include "NaiveLRU.h"
#include <algorithm>

NaiveLRU::NaiveLRU(int capacity)
    : capacity_(capacity)
    , hits_(0)
    , misses_(0)
{
    cache_.reserve(capacity);
}

int NaiveLRU::get(int key) {
    for (int i = 0; i < static_cast<int>(cache_.size()); ++i) {
        if (cache_[i].first == key) {
            ++hits_;
            int value = cache_[i].second;
            cache_.erase(cache_.begin() + i);
            cache_.insert(cache_.begin(), {key, value});
            return value;
        }
    }
    ++misses_;
    return -1;
}

void NaiveLRU::put(int key, int value) {
    for (int i = 0; i < static_cast<int>(cache_.size()); ++i) {
        if (cache_[i].first == key) {
            cache_.erase(cache_.begin() + i);
            cache_.insert(cache_.begin(), {key, value});
            return;
        }
    }
    
    if (static_cast<int>(cache_.size()) == capacity_) {
        cache_.pop_back();
    }
    cache_.insert(cache_.begin(), {key, value});
}

void NaiveLRU::resetStats() {
    hits_ = 0;
    misses_ = 0;
}

void NaiveLRU::clear() {
    cache_.clear();
    resetStats();
}

int NaiveLRU::size() const {
    return static_cast<int>(cache_.size());
}
