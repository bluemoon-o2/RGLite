// ListValue.h - List value implementation for RGLite
// This file implements the list value type for the VM

#ifndef RGLITE_LISTVALUE_H
#define RGLITE_LISTVALUE_H

#include <vector>
#include <memory>
#include <string>
#include "Bytecode.h"
#include "MemoryManager.h"

namespace rglite {

// Forward declaration
class MemoryManager;

// List value class
class ListValue {
public:
    // Constructor
    ListValue() = default;
    
    // Constructor with initial capacity
    explicit ListValue(size_t capacity) {
        items_.reserve(capacity);
    }
    
    // Reserve capacity for the list
    void reserve(size_t capacity) {
        items_.reserve(capacity);
    }
    
    // Constructor with initial items
    explicit ListValue(const std::vector<Value>& items) : items_(items) {}
    
    // Get the number of items in the list
    size_t size() const { return items_.size(); }
    
    // Get the capacity of the list
    size_t capacity() const { return items_.capacity(); }
    
    // Check if the list is empty
    bool empty() const { return items_.empty(); }
    
    // Get an item at the specified index
    Value get(size_t index) const {
        if (index >= items_.size()) {
            return Value(); // Return nil if index is out of bounds
        }
        return items_[index];
    }
    
    // Set an item at the specified index
    bool set(size_t index, const Value& value) {
        if (index >= items_.size()) {
            return false; // Index out of bounds
        }
        items_[index] = value;
        return true;
    }
    
    // Append an item to the list
    void append(const Value& value) {
        items_.push_back(value);
    }
    
    // Insert an item at the specified index
    bool insert(size_t index, const Value& value) {
        if (index > items_.size()) {
            return false; // Index out of bounds
        }
        items_.insert(items_.begin() + index, value);
        return true;
    }
    
    // Remove an item at the specified index
    bool remove(size_t index) {
        if (index >= items_.size()) {
            return false; // Index out of bounds
        }
        items_.erase(items_.begin() + index);
        return true;
    }
    
    // Get all items as a vector
    const std::vector<Value>& getItems() const { return items_; }
    
    // Convert to string representation
    std::string toString() const {
        std::string result = "[";
        for (size_t i = 0; i < items_.size(); ++i) {
            if (i > 0) result += ", ";
            result += valueToString(items_[i]);
        }
        result += "]";
        return result;
    }
    
private:
    std::vector<Value> items_;
};

// List storage for the VM
class ListStorage {
public:
    // Constructor
    ListStorage() = default;
    
    // Constructor with memory manager
    explicit ListStorage(MemoryManager* memoryManager) : memoryManager_(memoryManager) {}
    
    // Get the number of lists
    size_t getCount() const {
        return lists_.size();
    }
    
    // Get the number of allocated lists (alias for getCount)
    size_t getAllocatedLists() const {
        return getCount();
    }
    
    // Create a new list
    size_t createList() {
        // Always create the list ourselves
        lists_.push_back(std::make_unique<ListValue>());
        return lists_.size() - 1;
    }
    
    // Create a new list with initial capacity
    size_t createList(size_t capacity) {
        // Always create the list ourselves
        lists_.push_back(std::make_unique<ListValue>(capacity));
        return lists_.size() - 1;
    }
    
    // Create a new list with initial items
    size_t createList(const std::vector<Value>& items) {
        // Always create the list ourselves
        lists_.push_back(std::make_unique<ListValue>(items));
        return lists_.size() - 1;
    }
    
    // Get a list by index
    ListValue* getList(size_t index) {
        if (index >= lists_.size()) {
            return nullptr;
        }
        return lists_[index].get();
    }
    
    // Get a list by index (const version)
    const ListValue* getList(size_t index) const {
        if (index >= lists_.size()) {
            return nullptr;
        }
        return lists_[index].get();
    }
    
    // Clear all lists
    void clear() {
        lists_.clear();
    }
    
    // Set memory manager
    void setMemoryManager(MemoryManager* memoryManager) {
        memoryManager_ = memoryManager;
    }
    
private:
    std::vector<std::unique_ptr<ListValue>> lists_;
    MemoryManager* memoryManager_ = nullptr;
};

} // namespace rglite

#endif // RGLITE_LISTVALUE_H
