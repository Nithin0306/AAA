#pragma once

#ifndef FIFO_CACHE_H
#define FIFO_CACHE_H

#include <queue>
#include <unordered_map>

using namespace std;

class FIFOCache {
public:
    explicit FIFOCache(int capacity);

    int get(int key);
    void put(int key, int value);

    int hits() const { return hits_; }
    int misses() const { return misses_; }
    int total() const { return hits_ + misses_; }
    void resetStats();
    void clear();

    int size() const;
    int capacity() const { return capacity_; }

private:
    int capacity_;
    queue<int> order_;
    unordered_map<int,int> map_;

    int hits_ = 0;
    int misses_ = 0;
};

#endif
