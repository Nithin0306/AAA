#pragma once

#ifndef BELADY_CACHE_H
#define BELADY_CACHE_H

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <utility>
#include <limits>

using namespace std;

class BeladyCache {
public:
    static constexpr int INF = numeric_limits<int>::max();

    BeladyCache(int capacity, const vector<int>& sequence);

    pair<int,int> simulate();
    int access(int key, int currentIdx);

    int hits() const { return hits_; }
    int misses() const { return misses_; }
    int total() const { return hits_ + misses_; }
    void resetStats();
    void reset();

    int size() const;
    int capacity() const { return capacity_; }

private:
    void buildFutureMap();
    int findEvictionCandidate(int currentIdx);
    int nextUse(int key, int currentIdx) const;

    int capacity_;
    const vector<int> seq_;
    unordered_set<int> cacheSet_;
    unordered_map<int, queue<int>> futureMap_;

    int hits_ = 0;
    int misses_ = 0;
};

#endif
