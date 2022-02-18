#ifndef __CACHE_H__
#define __CACHE_H__

#include "settings.hpp"
#include "http_message.hpp"
#include <unordered_map>

struct DLinkedNode {
    std::string key;
    Response* value;
    DLinkedNode* prev;
    DLinkedNode* next;
    DLinkedNode(): key(""), value(NULL), prev(NULL), next(NULL) {}
    DLinkedNode(std::string _key, Response* _value): key(_key), value(_value), prev(NULL), next(NULL) {}
};

class LRUCache {
private:
    std::unordered_map<std::string, DLinkedNode*> cache;
    DLinkedNode* head;
    DLinkedNode* tail;
    int size;
    int capacity;

public:
    LRUCache(int _capacity): capacity(_capacity), size(0) {
        head = new DLinkedNode();
        tail = new DLinkedNode();
        head->next = tail;
        tail->prev = head;
    }
    
    Response* get(std::string key) {
        if (!cache.count(key)) {
            return NULL;
        }
        DLinkedNode* node = cache[key];
        moveToHead(node);
        return node->value;
    }
    
    void put(std::string key, Response* value) {
        if (!cache.count(key)) {
            DLinkedNode* node = new DLinkedNode(key, value);
            cache[key] = node;
            addToHead(node);
            ++size;
            if (size > capacity) {
                DLinkedNode* removed = removeTail();
                cache.erase(removed->key);
                delete removed;
                --size;
            }
        }
        else {
            DLinkedNode* node = cache[key];
            node->value = value;
            moveToHead(node);
        }
    }

    void addToHead(DLinkedNode* node) {
        node->prev = head;
        node->next = head->next;
        head->next->prev = node;
        head->next = node;
    }
    
    void removeNode(DLinkedNode* node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    void moveToHead(DLinkedNode* node) {
        removeNode(node);
        addToHead(node);
    }

    DLinkedNode* removeTail() {
        DLinkedNode* node = tail->prev;
        proxy_log << "(no-id): NOTE evicted " << node->key << " from cache" << std::endl;
        removeNode(node);
        return node;
    }
};

#endif