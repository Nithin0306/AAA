#include "SLRU.h"
#include <algorithm>

using namespace std;

SLRU::Segment::Segment(int capacity) : cap(capacity) {
    head = new Node(-1, -1);
    tail = new Node(-1, -1);
    head->next = tail;
    tail->prev = head;
}

SLRU::Segment::~Segment() {
    Node* curr = head;
    while (curr != nullptr) {
        Node* next = curr->next;
        delete curr;
        curr = next;
    }
}

void SLRU::Segment::addToFront(Node* node) {
    node->prev = head;
    node->next = head->next;
    head->next->prev = node;
    head->next = node;
}

void SLRU::Segment::removeNode(Node* node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = nullptr;
    node->next = nullptr;
}

SLRU::Node* SLRU::Segment::evictLRU() {
    if (map.empty()) return nullptr;
    Node* lru = tail->prev;
    removeNode(lru);
    map.erase(lru->key);
    return lru;
}

void SLRU::Segment::insertNode(Node* node) {
    map[node->key] = node;
    addToFront(node);
}

SLRU::SLRU(int totalCapacity)
    : probation_(max(1, totalCapacity / 5))
    , protected_(totalCapacity - max(1, totalCapacity / 5))
    , probCap_(max(1, totalCapacity / 5))
    , protCap_(totalCapacity - max(1, totalCapacity / 5))
    , hits_(0)
    , misses_(0)
{}

SLRU::~SLRU() {}

int SLRU::get(int key) {
    if (protected_.contains(key)) {
        ++hits_;
        Node* node = protected_.map[key];
        protected_.removeNode(node);
        protected_.addToFront(node);
        return node->value;
    }

    if (probation_.contains(key)) {
        ++hits_;
        Node* node = probation_.map[key];
        probation_.removeNode(node);
        probation_.map.erase(key);

        // promote to protected; if protected is full, demote its LRU to probation
        if (protected_.currentSize() == protected_.cap) {
            Node* demoted = protected_.evictLRU();
            if (demoted != nullptr) {
                if (probation_.currentSize() == probation_.cap) {
                    Node* evicted = probation_.evictLRU();
                    delete evicted;
                }
                probation_.insertNode(demoted);
            }
        }
        protected_.insertNode(node);
        return node->value;
    }

    ++misses_;
    return -1;
}

void SLRU::put(int key, int value) {
    if (protected_.contains(key)) {
        Node* node = protected_.map[key];
        node->value = value;
        protected_.removeNode(node);
        protected_.addToFront(node);
        return;
    }

    if (probation_.contains(key)) {
        Node* node = probation_.map[key];
        node->value = value;
        probation_.removeNode(node);
        probation_.map.erase(key);

        if (protected_.currentSize() == protected_.cap) {
            Node* demoted = protected_.evictLRU();
            if (demoted != nullptr) {
                if (probation_.currentSize() == probation_.cap) {
                    Node* evicted = probation_.evictLRU();
                    delete evicted;
                }
                probation_.insertNode(demoted);
            }
        }
        protected_.insertNode(node);
        return;
    }

    // new item goes to probation
    if (probation_.currentSize() == probation_.cap) {
        Node* evicted = probation_.evictLRU();
        delete evicted;
    }
    Node* newNode = new Node(key, value);
    probation_.insertNode(newNode);
}

void SLRU::resetStats() {
    hits_ = 0;
    misses_ = 0;
}

void SLRU::clear() {
    while (probation_.currentSize() > 0) {
        Node* n = probation_.evictLRU();
        delete n;
    }
    while (protected_.currentSize() > 0) {
        Node* n = protected_.evictLRU();
        delete n;
    }
    resetStats();
}

int SLRU::size() const {
    return probation_.currentSize() + protected_.currentSize();
}
