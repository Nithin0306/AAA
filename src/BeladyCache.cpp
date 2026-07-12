#include "BeladyCache.h"
#include <algorithm>

BeladyCache::BeladyCache(int capacity, const vector<int>& sequence)
    : capacity_(capacity)
    , seq_(sequence)
    , hits_(0)
    , misses_(0)
{
    buildFutureMap();
}

void BeladyCache::buildFutureMap() {
    for (int i = 0; i < static_cast<int>(seq_.size()); ++i) {
        futureMap_[seq_[i]].push(i);
    }
}

int BeladyCache::nextUse(int key, int currentIdx) const {
    auto it = futureMap_.find(key);
    if (it == futureMap_.end()) return INF;
    const auto& q = it->second;
    
    if (q.empty()) return INF;
    return q.front() > currentIdx ? q.front() : INF;
}

int BeladyCache::findEvictionCandidate(int currentIdx) {
    int farthestKey = -1;
    int farthestDist = -1;

    for (int key : cacheSet_) {
        int dist = INF;
        auto it = futureMap_.find(key);
        if (it != futureMap_.end()) {
            auto& q = it->second;
            while (!q.empty() && q.front() <= currentIdx) q.pop();
            dist = q.empty() ? INF : q.front();
        }
        
        if (dist > farthestDist || (dist == farthestDist && key < farthestKey)) {
            farthestDist = dist;
            farthestKey = key;
        }
    }
    return farthestKey;
}

int BeladyCache::access(int key, int currentIdx) {
    auto it = futureMap_.find(key);
    if (it != futureMap_.end()) {
        auto& q = it->second;
        while (!q.empty() && q.front() <= currentIdx) q.pop();
    }

    if (cacheSet_.count(key)) {
        ++hits_;
        return 1;
    }

    ++misses_;
    if (static_cast<int>(cacheSet_.size()) == capacity_) {
        int victim = findEvictionCandidate(currentIdx);
        cacheSet_.erase(victim);
    }
    cacheSet_.insert(key);
    return 0;
}

pair<int,int> BeladyCache::simulate() {
    reset();
    for (int i = 0; i < static_cast<int>(seq_.size()); ++i) {
        access(seq_[i], i);
    }
    return {hits_, misses_};
}

void BeladyCache::resetStats() {
    hits_ = 0;
    misses_ = 0;
}

void BeladyCache::reset() {
    cacheSet_.clear();
    futureMap_.clear();
    buildFutureMap();
    resetStats();
}

int BeladyCache::size() const {
    return static_cast<int>(cacheSet_.size());
}
