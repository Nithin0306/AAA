#pragma once

#ifndef SLRU_H
#define SLRU_H

#include <unordered_map>
#include <unordered_set>

using namespace std;

// Segmented LRU: splits capacity into Probation (20%) and Protected (80%).
// New items enter Probation. A second cache hit promotes them to Protected.
// Scan items never leave Probation, so they can't evict hot working-set data.

class SLRU {
public:
    explicit SLRU(int totalCapacity);
    ~SLRU();

    SLRU(const SLRU&) = delete;
    SLRU& operator=(const SLRU&) = delete;

    int get(int key);
    void put(int key, int value);

    int hits() const { return hits_; }
    int misses() const { return misses_; }
    int total() const { return hits_ + misses_; }

    void resetStats();
    void clear();

    int size() const;
    int capacity() const { return probCap_ + protCap_; }

private:
    struct Node {
        int key, value;
        Node* prev;
        Node* next;
        Node(int k, int v) : key(k), value(v), prev(nullptr), next(nullptr) {}
    };

    struct Segment {
        int cap;
        unordered_map<int, Node*> map;
        Node* head;
        Node* tail;

        explicit Segment(int capacity);
        ~Segment();

        void addToFront(Node* node);
        void removeNode(Node* node);
        bool contains(int key) const { return map.count(key) > 0; }
        int currentSize() const { return static_cast<int>(map.size()); }
        Node* evictLRU();
        void insertNode(Node* node);
    };

    Segment probation_;
    Segment protected_;

    int probCap_;
    int protCap_;

    int hits_ = 0;
    int misses_ = 0;
};

#endif
