// SetValue.h - Set value implementation for RGLite
// This file implements the set value type for the VM

#ifndef RGLITE_SETVALUE_H
#define RGLITE_SETVALUE_H

#include <unordered_set>
#include <memory>
#include <string>
#include "Bytecode.h"
#include "MemoryManager.h"

namespace rglite {

// Forward declaration
class MemoryManager;

// Hash function for Value objects
struct ValueHash {
    size_t operator()(const Value& value) const {
        switch (value.getType()) {
            case ValueType::NIL:
                return std::hash<int>{}(0);
            case ValueType::BOOLEAN:
                return std::hash<bool>{}(value.asBoolean());
            case ValueType::INTEGER:
                return std::hash<int64_t>{}(value.asInteger());
            case ValueType::FLOAT:
                return std::hash<double>{}(value.asFloat());
            case ValueType::STRING:
                return std::hash<std::string>{}(value.asString());
            case ValueType::LIST:
            case ValueType::DICT:
            case ValueType::TUPLE:
            case ValueType::FUNCTION:
            case ValueType::NATIVE_FUNCTION:
            case ValueType::ITERATOR:
            case ValueType::EXCEPTION:
                // For complex types, use the type and index
                return std::hash<int>{}(static_cast<int>(value.getType())) ^ 
                       std::hash<uint32_t>{}(value.asIndex());
            default:
                return 0;
        }
    }
};

// Equality function for Value objects
struct ValueEqual {
    bool operator()(const Value& a, const Value& b) const {
        return Value::valuesEqual(a, b);
    }
};

// Set value class
class SetValue {
public:
    // Constructor
    SetValue() = default;
    
    // Reserve capacity for the set
    void reserve(size_t capacity) {
        items_.reserve(capacity);
    }
    
    // Constructor with initial items
    explicit SetValue(const std::vector<Value>& items) {
        for (const auto& item : items) {
            items_.insert(item);
        }
    }
    
    // Get the number of items in the set
    size_t size() const { return items_.size(); }
    
    // Check if the set is empty
    bool empty() const { return items_.empty(); }
    
    // Check if the set contains a value
    bool contains(const Value& value) const {
        return items_.find(value) != items_.end();
    }
    
    // Add a value to the set
    bool add(const Value& value) {
        auto result = items_.insert(value);
        return result.second;  // true if insertion took place
    }
    
    // Remove a value from the set
    bool remove(const Value& value) {
        return items_.erase(value) > 0;
    }
    
    // Clear all items from the set
    void clear() {
        items_.clear();
    }
    
    // Get all items as a vector
    std::vector<Value> getItems() const {
        std::vector<Value> result;
        result.reserve(items_.size());
        for (const auto& item : items_) {
            result.push_back(item);
        }
        return result;
    }
    
    // Convert to string representation
    std::string toString() const {
        std::string result = "{";
        bool first = true;
        for (const auto& item : items_) {
            if (!first) {
                result += ", ";
            }
            result += valueToString(item);
            first = false;
        }
        result += "}";
        return result;
    }
    
    // Set union
    std::unique_ptr<SetValue> unionWith(const SetValue& other) const {
        auto result = std::make_unique<SetValue>();
        result->items_ = items_;
        result->items_.insert(other.items_.begin(), other.items_.end());
        return result;
    }
    
    // Set intersection
    std::unique_ptr<SetValue> intersectionWith(const SetValue& other) const {
        auto result = std::make_unique<SetValue>();
        for (const auto& item : items_) {
            if (other.items_.find(item) != other.items_.end()) {
                result->items_.insert(item);
            }
        }
        return result;
    }
    
    // Set difference
    std::unique_ptr<SetValue> differenceWith(const SetValue& other) const {
        auto result = std::make_unique<SetValue>();
        for (const auto& item : items_) {
            if (other.items_.find(item) == other.items_.end()) {
                result->items_.insert(item);
            }
        }
        return result;
    }
    
    // Check if this set is a subset of another
    bool isSubsetOf(const SetValue& other) const {
        for (const auto& item : items_) {
            if (other.items_.find(item) == other.items_.end()) {
                return false;
            }
        }
        return true;
    }
    
    // Check if this set is a superset of another
    bool isSupersetOf(const SetValue& other) const {
        return other.isSubsetOf(*this);
    }
    
private:
    std::unordered_set<Value, ValueHash, ValueEqual> items_;
};

// Set storage for the VM
class SetStorage {
public:
    // Constructor
    SetStorage() = default;
    
    // Constructor with memory manager
    explicit SetStorage(MemoryManager* memoryManager) : memoryManager_(memoryManager) {}
    
    // Get the number of sets
    size_t getCount() const {
        return sets_.size();
    }
    
    // Get the number of allocated sets (alias for getCount)
    size_t getAllocatedSets() const {
        return getCount();
    }
    
    // Create a new set
    size_t createSet() {
        // Always create the set ourselves
        sets_.push_back(std::make_unique<SetValue>());
        return sets_.size() - 1;
    }
    
    // Create a new set with initial capacity
    size_t createSet(size_t capacity) {
        auto set = std::make_unique<SetValue>();
        set->reserve(capacity);
        sets_.push_back(std::move(set));
        return sets_.size() - 1;
    }
    
    // Create a new set with initial items
    size_t createSet(const std::vector<Value>& items) {
        // Always create the set ourselves
        sets_.push_back(std::make_unique<SetValue>(items));
        return sets_.size() - 1;
    }
    
    // Get a set by index
    SetValue* getSet(size_t index) {
        if (index >= sets_.size()) {
            return nullptr;
        }
        return sets_[index].get();
    }
    
    // Get a set by index (const version)
    const SetValue* getSet(size_t index) const {
        if (index >= sets_.size()) {
            return nullptr;
        }
        return sets_[index].get();
    }
    
    // Clear all sets
    void clear() {
        sets_.clear();
    }
    
    // Set memory manager
    void setMemoryManager(MemoryManager* memoryManager) {
        memoryManager_ = memoryManager;
    }
    
private:
    std::vector<std::unique_ptr<SetValue>> sets_;
    MemoryManager* memoryManager_ = nullptr;
};

} // namespace rglite

#endif // RGLITE_SETVALUE_H