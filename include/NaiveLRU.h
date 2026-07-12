#pragma once

#ifndef NAIVE_LRU_H
#define NAIVE_LRU_H

#include <vector>
#include <utility>

using namespace std;

class NaiveLRU {
public:
    explicit NaiveLRU(int capacity);

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
    vector<pair<int, int>> cache_;

    int hits_ = 0;
    int misses_ = 0;
};

#endif
