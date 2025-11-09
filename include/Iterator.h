// Iterator.h - Iterator implementation for RGLite
// This file implements the iterator for containers in RGLite

#ifndef RGLITE_ITERATOR_H
#define RGLITE_ITERATOR_H

#include "Bytecode.h"
#include "ListValue.h"
#include "DictValue.h"
#include "TupleValue.h"
#include "SetValue.h"
#include <memory>
#include <variant>

namespace rglite {

// Forward declaration
class MemoryManager;

// Iterator types
enum class IteratorType : uint8_t {
    LIST_ITERATOR,
    DICT_KEY_ITERATOR,
    DICT_VALUE_ITERATOR,
    DICT_ITEM_ITERATOR,
    TUPLE_ITERATOR,
    SET_ITERATOR
};

// Iterator class for containers
class Iterator {
public:
    // Constructor for list iterator
    Iterator(ListValue* list, size_t startIndex = 0) 
        : type_(IteratorType::LIST_ITERATOR), currentIndex_(startIndex) {
        list_ = list;
    }
    
    // Constructor for dictionary key iterator
    Iterator(DictValue* dict, IteratorType type, size_t startIndex = 0) 
        : type_(type), currentIndex_(startIndex) {
        dict_ = dict;
        if (type != IteratorType::DICT_KEY_ITERATOR && 
            type != IteratorType::DICT_VALUE_ITERATOR && 
            type != IteratorType::DICT_ITEM_ITERATOR) {
            type_ = IteratorType::DICT_KEY_ITERATOR; // Default to key iterator
        }
    }
    
    // Constructor for tuple iterator
    Iterator(TupleValue* tuple, size_t startIndex = 0) 
        : type_(IteratorType::TUPLE_ITERATOR), currentIndex_(startIndex) {
        tuple_ = tuple;
    }
    
    // Constructor for set iterator
    Iterator(SetValue* set, size_t startIndex = 0) 
        : type_(IteratorType::SET_ITERATOR), currentIndex_(startIndex) {
        set_ = set;
    }
    
    // Check if iterator has more elements
    bool hasNext() const {
        if (type_ == IteratorType::LIST_ITERATOR) {
            return list_ && currentIndex_ < list_->size();
        } else if (type_ == IteratorType::DICT_KEY_ITERATOR ||
                   type_ == IteratorType::DICT_VALUE_ITERATOR ||
                   type_ == IteratorType::DICT_ITEM_ITERATOR) {
            return dict_ && currentIndex_ < dict_->size();
        } else if (type_ == IteratorType::TUPLE_ITERATOR) {
            return tuple_ && currentIndex_ < tuple_->size();
        } else if (type_ == IteratorType::SET_ITERATOR) {
            return set_ && currentIndex_ < set_->size();
        }
        return false;
    }
    
    // Get next element
    Value next() {
        if (!hasNext()) {
            return Value(); // Return nil if no more elements
        }
        
        if (type_ == IteratorType::LIST_ITERATOR) {
            Value result = list_->get(currentIndex_++);
            return result;
        } else if (type_ == IteratorType::DICT_KEY_ITERATOR) {
            auto keys = dict_->getKeys();
            Value result = Value(keys[currentIndex_++]);
            return result;
        } else if (type_ == IteratorType::DICT_VALUE_ITERATOR) {
            auto values = dict_->getValues();
            Value result = values[currentIndex_++];
            return result;
        } else if (type_ == IteratorType::DICT_ITEM_ITERATOR) {
            // For item iterator, we need to create a list containing key and value
            auto items = dict_->getItems();
            auto& item = items[currentIndex_++];
            
            // Return a string representation of key-value pair for now
            // In a full implementation, we would create a proper list object
            std::string result = item.first + ":";
            
            // Convert the value to string based on its type
            if (item.second.isNil()) {
                result += "nil";
            } else if (item.second.isBoolean()) {
                result += (item.second.asBoolean() ? "true" : "false");
            } else if (item.second.isInteger()) {
                result += std::to_string(item.second.asInteger());
            } else if (item.second.isFloat()) {
                result += std::to_string(item.second.asFloat());
            } else if (item.second.isString()) {
                result += item.second.asString();
            } else {
                result += "?"; // Unknown type
            }
            
            Value stringResult = Value(result);
            return stringResult;
        } else if (type_ == IteratorType::TUPLE_ITERATOR) {
            Value result = tuple_->get(currentIndex_++);
            return result;
        } else if (type_ == IteratorType::SET_ITERATOR) {
            // For set iterator, we need to get the items and return the current one
            auto items = set_->getItems();
            Value result = items[currentIndex_++];
            return result;
        }
        return Value(); // Default case
    }
    
    // Get iterator type
    IteratorType getType() const { return type_; }
    
    // Get current index
    size_t getCurrentIndex() const { return currentIndex_; }
    
private:
    IteratorType type_;
    size_t currentIndex_;
    ListValue* list_ = nullptr;
    DictValue* dict_ = nullptr;
    TupleValue* tuple_ = nullptr;
    SetValue* set_ = nullptr;
};

// Iterator storage for the VM
class IteratorStorage {
public:
    // Constructor
    IteratorStorage() = default;
    
    // Constructor with memory manager
    explicit IteratorStorage(MemoryManager* memoryManager) : memoryManager_(memoryManager) {}
    
    // Get the number of iterators
    size_t getCount() const {
        return iterators_.size();
    }
    
    // Get the number of allocated iterators (alias for getCount)
    size_t getAllocatedIterators() const {
        return getCount();
    }
    
    // Create a new list iterator
    size_t createListIterator(ListValue* list, size_t startIndex = 0) {
        iterators_.push_back(std::make_unique<Iterator>(list, startIndex));
        return iterators_.size() - 1;
    }
    
    // Create a new dictionary iterator
    size_t createDictIterator(DictValue* dict, IteratorType type, size_t startIndex = 0) {
        iterators_.push_back(std::make_unique<Iterator>(dict, type, startIndex));
        return iterators_.size() - 1;
    }
    
    // Create a new tuple iterator
    size_t createTupleIterator(TupleValue* tuple, size_t startIndex = 0) {
        iterators_.push_back(std::make_unique<Iterator>(tuple, startIndex));
        return iterators_.size() - 1;
    }
    
    // Create a new set iterator
    size_t createSetIterator(SetValue* set, size_t startIndex = 0) {
        iterators_.push_back(std::make_unique<Iterator>(set, startIndex));
        return iterators_.size() - 1;
    }
    
    // Get an iterator by index
    Iterator* getIterator(size_t index) {
        if (index >= iterators_.size()) {
            return nullptr;
        }
        return iterators_[index].get();
    }
    
    // Clear all iterators
    void clear() {
        iterators_.clear();
    }
    
    // Set memory manager
    void setMemoryManager(MemoryManager* memoryManager) {
        memoryManager_ = memoryManager;
    }
    
private:
    std::vector<std::unique_ptr<Iterator>> iterators_;
    MemoryManager* memoryManager_ = nullptr;
};

} // namespace rglite

#endif // RGLITE_ITERATOR_H
