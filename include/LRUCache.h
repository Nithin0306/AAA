#pragma once

#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <unordered_map>

using namespace std;

class LRUCache {
public:
    explicit LRUCache(int capacity);
    ~LRUCache();

    LRUCache(const LRUCache&) = delete;
    LRUCache& operator=(const LRUCache&) = delete;

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
    struct Node {
        int key, value;
        Node* prev;
        Node* next;
        Node(int k, int v)
            : key(k), value(v), prev(nullptr), next(nullptr) {}
    };

    void removeNode(Node* node);
    void addToFront(Node* node);

    int capacity_;
    unordered_map<int, Node*> map_;

    // dummy sentinels
    Node* head_;
    Node* tail_;

    int hits_ = 0;
    int misses_ = 0;
};

#endif
