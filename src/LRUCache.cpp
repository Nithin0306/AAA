#include "LRUCache.h"

LRUCache::LRUCache(int capacity)
    : capacity_(capacity)
    , hits_(0)
    , misses_(0)
{
    head_ = new Node(-1, -1);
    tail_ = new Node(-1, -1);
    head_->next = tail_;
    tail_->prev = head_;
}

LRUCache::~LRUCache() {
    Node* curr = head_;
    while (curr != nullptr) {
        Node* next = curr->next;
        delete curr;
        curr = next;
    }
}

void LRUCache::removeNode(Node* node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

void LRUCache::addToFront(Node* node) {
    node->prev = head_;
    node->next = head_->next;
    head_->next->prev = node;
    head_->next = node;
}

int LRUCache::get(int key) {
    auto it = map_.find(key);
    if (it == map_.end()) {
        ++misses_;
        return -1;
    }
    
    Node* node = it->second;
    removeNode(node);
    addToFront(node);
    ++hits_;
    
    return node->value;
}

void LRUCache::put(int key, int value) {
    auto it = map_.find(key);
    if (it != map_.end()) {
        Node* node = it->second;
        node->value = value;
        removeNode(node);
        addToFront(node);
    } else {
        if (static_cast<int>(map_.size()) == capacity_) {
            Node* lru = tail_->prev;
            map_.erase(lru->key);
            removeNode(lru);
            delete lru;
        }
        Node* newNode = new Node(key, value);
        map_[key] = newNode;
        addToFront(newNode);
    }
}

void LRUCache::resetStats() {
    hits_ = 0;
    misses_ = 0;
}

void LRUCache::clear() {
    Node* curr = head_->next;
    while (curr != tail_) {
        Node* next = curr->next;
        delete curr;
        curr = next;
    }
    head_->next = tail_;
    tail_->prev = head_;
    map_.clear();
    resetStats();
}

int LRUCache::size() const {
    return static_cast<int>(map_.size());
}
