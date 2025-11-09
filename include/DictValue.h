// DictValue.h - Dictionary value implementation for RGLite
// This file implements the dictionary value type for the VM

#ifndef RGLITE_DICTVALUE_H
#define RGLITE_DICTVALUE_H

#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <iterator>
#include "Bytecode.h"
#include "MemoryManager.h"

namespace rglite {

// Forward declaration
class MemoryManager;

// Dictionary value class
class DictValue {
public:
    // Constructor
    DictValue() = default;
    
    // Constructor with initial capacity
    explicit DictValue(size_t capacity) {
        items_.reserve(capacity);
    }
    
    // Reserve capacity for the dictionary
    void reserve(size_t capacity) {
        items_.reserve(capacity);
    }
    
    // Constructor with initial items
    explicit DictValue(const std::vector<std::pair<std::string, Value>>& items) {
        for (const auto& item : items) {
            items_[item.first] = item.second;
        }
    }
    
    // Get the number of items in the dictionary
    size_t size() const { return items_.size(); }
    
    // Get the number of buckets in the dictionary
    size_t bucket_count() const { return items_.bucket_count(); }
    
    // Check if the dictionary is empty
    bool empty() const { return items_.empty(); }
    
    // Check if the dictionary contains a key
    bool contains(const std::string& key) const {
        return items_.find(key) != items_.end();
    }
    
    // Get an item by key
    Value get(const std::string& key) const {
        auto it = items_.find(key);
        if (it == items_.end()) {
            return Value(); // Return nil if key not found
        }
        return it->second;
    }
    
    // Get an item by key with default value
    Value get(const std::string& key, const Value& defaultValue) const {
        auto it = items_.find(key);
        if (it == items_.end()) {
            return defaultValue;
        }
        return it->second;
    }
    
    // Set an item by key
    void set(const std::string& key, const Value& value) {
        items_[key] = value;
    }
    
    // Remove an item by key
    bool remove(const std::string& key) {
        auto it = items_.find(key);
        if (it == items_.end()) {
            return false; // Key not found
        }
        items_.erase(it);
        return true;
    }
    
    // Get all keys as a vector
    std::vector<std::string> getKeys() const {
        std::vector<std::string> keys;
        keys.reserve(items_.size());
        
        for (const auto& item : items_) {
            keys.push_back(item.first);
        }
        
        return keys;
    }
    
    // Get all values as a vector
    std::vector<Value> getValues() const {
        std::vector<Value> values;
        values.reserve(items_.size());
        
        for (const auto& item : items_) {
            values.push_back(item.second);
        }
        
        return values;
    }
    
    // Get all key-value pairs as a vector
    std::vector<std::pair<std::string, Value>> getItems() const {
        std::vector<std::pair<std::string, Value>> items;
        items.reserve(items_.size());
        
        for (const auto& item : items_) {
            items.emplace_back(item.first, item.second);
        }
        
        return items;
    }
    
    // Get key by position (for iterator support)
    Value getKey(size_t position) const {
        if (position >= items_.size()) {
            return Value(); // Return nil if position is out of range
        }
        
        auto it = items_.begin();
        std::advance(it, position);
        return Value(it->first);
    }
    
    // Get value by position (for iterator support)
    Value getValue(size_t position) const {
        if (position >= items_.size()) {
            return Value(); // Return nil if position is out of range
        }
        
        auto it = items_.begin();
        std::advance(it, position);
        return it->second;
    }
    
    // Get size (alias for size() for consistency with ListValue)
    size_t getSize() const {
        return items_.size();
    }
    
    // Clear all items
    void clear() {
        items_.clear();
    }
    
    // Convert to string representation
    std::string toString() const {
        std::string result = "{";
        bool first = true;
        
        for (const auto& item : items_) {
            if (!first) {
                result += ", ";
            }
            result += "\"";
            result += item.first;
            result += "\": ";
            result += valueToString(item.second);
            first = false;
        }
        
        result += "}";
        return result;
    }
    
private:
    std::unordered_map<std::string, Value> items_;
};

// Dictionary storage for the VM
class DictStorage {
public:
    // Constructor
    DictStorage() = default;
    
    // Constructor with memory manager
    explicit DictStorage(MemoryManager* memoryManager) : memoryManager_(memoryManager) {}
    
    // Get the number of dictionaries
    size_t getCount() const {
        return dicts_.size();
    }
    
    // Get the number of allocated dictionaries (alias for getCount)
    size_t getAllocatedDicts() const {
        return getCount();
    }
    
    // Create a new dictionary
    size_t createDict() {
        // Always create the dict ourselves
        dicts_.push_back(std::make_unique<DictValue>());
        return dicts_.size() - 1;
    }
    
    // Create a new dictionary with initial capacity
    size_t createDict(size_t capacity) {
        // Always create the dict ourselves
        dicts_.push_back(std::make_unique<DictValue>());
        dicts_.back()->reserve(capacity);
        return dicts_.size() - 1;
    }
    
    // Create a new dictionary with initial items
    size_t createDict(const std::vector<std::pair<std::string, Value>>& items) {
        // Always create the dict ourselves
        dicts_.push_back(std::make_unique<DictValue>(items));
        return dicts_.size() - 1;
    }
    
    // Get a dictionary by index
    DictValue* getDict(size_t index) {
        if (index >= dicts_.size()) {
            return nullptr;
        }
        return dicts_[index].get();
    }
    
    // Get a dictionary by index (const version)
    const DictValue* getDict(size_t index) const {
        if (index >= dicts_.size()) {
            return nullptr;
        }
        return dicts_[index].get();
    }
    
    // Clear all dictionaries
    void clear() {
        dicts_.clear();
    }
    
    // Set memory manager
    void setMemoryManager(MemoryManager* memoryManager) {
        memoryManager_ = memoryManager;
    }
    
private:
    std::vector<std::unique_ptr<DictValue>> dicts_;
    MemoryManager* memoryManager_ = nullptr;
};

} // namespace rglite

#endif // RGLITE_DICTVALUE_H