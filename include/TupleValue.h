// TupleValue.h - Tuple value implementation for RGLite
// This file implements the tuple value type for the VM

#ifndef RGLITE_TUPLEVALUE_H
#define RGLITE_TUPLEVALUE_H

#include <vector>
#include <memory>
#include <string>
#include "Bytecode.h"
#include "MemoryManager.h"

namespace rglite {

// Forward declaration
class MemoryManager;

// Tuple value class (immutable list)
class TupleValue {
public:
    // Constructor
    TupleValue() = default;
    
    // Constructor with initial capacity
    explicit TupleValue(size_t capacity) {
        items_.reserve(capacity);
    }
    
    // Reserve capacity for the tuple
    void reserve(size_t capacity) {
        items_.reserve(capacity);
    }
    
    // Constructor with initial items
    explicit TupleValue(const std::vector<Value>& items) : items_(items) {}
    
    // Get the number of items in the tuple
    size_t size() const { return items_.size(); }
    
    // Get the capacity of the tuple
    size_t capacity() const { return items_.capacity(); }
    
    // Check if the tuple is empty
    bool empty() const { return items_.empty(); }
    
    // Get an item at the specified index
    Value get(size_t index) const {
        if (index >= items_.size()) {
            return Value(); // Return nil if index is out of bounds
        }
        return items_[index];
    }
    
    // Get all items as a vector (const)
    const std::vector<Value>& getItems() const { return items_; }
    
    // Append an item to the tuple (used during construction)
    void append(const Value& item) {
        items_.push_back(item);
    }
    
    // Convert to string representation
    std::string toString() const {
        std::string result = "(";
        for (size_t i = 0; i < items_.size(); ++i) {
            if (i > 0) result += ", ";
            result += valueToString(items_[i]);
        }
        if (items_.size() == 1) {
            result += ",)";  // Single-element tuple needs a trailing comma
        } else {
            result += ")";
        }
        return result;
    }
    
    // Check if two tuples are equal
    bool equals(const TupleValue& other) const {
        if (items_.size() != other.items_.size()) {
            return false;
        }
        
        for (size_t i = 0; i < items_.size(); ++i) {
            if (!Value::valuesEqual(items_[i], other.items_[i])) {
                return false;
            }
        }
        
        return true;
    }
    
private:
    std::vector<Value> items_;
};

// Tuple storage for the VM
class TupleStorage {
public:
    // Constructor
    TupleStorage() = default;
    
    // Constructor with memory manager
    explicit TupleStorage(MemoryManager* memoryManager) : memoryManager_(memoryManager) {}
    
    // Get the number of tuples
    size_t getCount() const {
        return tuples_.size();
    }
    
    // Get the number of allocated tuples (alias for getCount)
    size_t getAllocatedTuples() const {
        return getCount();
    }
    
    // Create a new tuple
    size_t createTuple() {
        // Always create the tuple ourselves
        tuples_.push_back(std::make_unique<TupleValue>());
        return tuples_.size() - 1;
    }
    
    // Create a new tuple with initial capacity
    size_t createTuple(size_t capacity) {
        // Always create the tuple ourselves
        tuples_.push_back(std::make_unique<TupleValue>(capacity));
        return tuples_.size() - 1;
    }
    
    // Create a new tuple with initial items
    size_t createTuple(const std::vector<Value>& items) {
        // Always create the tuple ourselves
        tuples_.push_back(std::make_unique<TupleValue>(items));
        return tuples_.size() - 1;
    }
    
    // Get a tuple by index
    TupleValue* getTuple(size_t index) {
        if (index >= tuples_.size()) {
            return nullptr;
        }
        return tuples_[index].get();
    }
    
    // Get a tuple by index (const version)
    const TupleValue* getTuple(size_t index) const {
        if (index >= tuples_.size()) {
            return nullptr;
        }
        return tuples_[index].get();
    }
    
    // Clear all tuples
    void clear() {
        tuples_.clear();
    }
    
    // Set memory manager
    void setMemoryManager(MemoryManager* memoryManager) {
        memoryManager_ = memoryManager;
    }
    
private:
    std::vector<std::unique_ptr<TupleValue>> tuples_;
    MemoryManager* memoryManager_ = nullptr;
};

} // namespace rglite

#endif // RGLITE_TUPLEVALUE_H