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
#include <functional>

namespace rglite {

// Forward declaration
class MemoryManager;
class VM;

// Iterator types
enum class IteratorType : uint8_t {
    LIST_ITERATOR,
    DICT_KEY_ITERATOR,
    DICT_VALUE_ITERATOR,
    DICT_ITEM_ITERATOR,
    TUPLE_ITERATOR,
    SET_ITERATOR,
    STRING_ITERATOR,
    ENUMERATE_ITERATOR,
    ZIP_ITERATOR,
    MAP_ITERATOR,
    FILTER_ITERATOR
};

// Iterator class for containers
class Iterator {
public:
    // Constructor for wrapper iterators needs VM
    explicit Iterator(VM* vm) : vm_(vm) {}

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

    // Constructor for string iterator (iterates characters)
    Iterator(const std::string& str, size_t startIndex = 0)
        : type_(IteratorType::STRING_ITERATOR), currentIndex_(startIndex), string_(str) {}

    // Constructor for enumerate wrapper iterator
    Iterator(VM* vm, Iterator* base, int64_t startIndex)
        : type_(IteratorType::ENUMERATE_ITERATOR), currentIndex_(0), vm_(vm), enumerateBase_(base), enumerateIndex_(startIndex) {}

    // Constructor for zip wrapper iterator
    Iterator(VM* vm, const std::vector<Iterator*>& bases)
        : type_(IteratorType::ZIP_ITERATOR), currentIndex_(0), vm_(vm), zipBases_(bases) {}

    // Constructor for map wrapper iterator
    Iterator(VM* vm, Iterator* base, std::function<Value(const Value&)> mapFunc)
        : type_(IteratorType::MAP_ITERATOR), currentIndex_(0), vm_(vm), mapBase_(base), mapFunc_(std::move(mapFunc)) {}

    // Constructor for filter wrapper iterator
    Iterator(VM* vm, Iterator* base, std::function<bool(const Value&)> predFunc)
        : type_(IteratorType::FILTER_ITERATOR), currentIndex_(0), vm_(vm), filterBase_(base), filterPred_(std::move(predFunc)) {}
    
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
        } else if (type_ == IteratorType::STRING_ITERATOR) {
            return currentIndex_ < string_.size();
        } else if (type_ == IteratorType::ENUMERATE_ITERATOR) {
            return enumerateBase_ && enumerateBase_->hasNext();
        } else if (type_ == IteratorType::ZIP_ITERATOR) {
            if (zipBases_.empty()) return false;
            for (auto* it : zipBases_) { if (!it || !it->hasNext()) return false; }
            return true;
        } else if (type_ == IteratorType::MAP_ITERATOR) {
            return mapBase_ && mapBase_->hasNext();
        } else if (type_ == IteratorType::FILTER_ITERATOR) {
            // Need to check if any further element satisfies predicate; but for hasNext we do a lookahead.
            // Implement simple lookahead buffering: if filterHasBuffered_ then true; else advance until match or exhaustion.
            if (!filterBase_) return false;
            if (filterHasBuffered_) return true;
            // mutable cast to update buffer in const method
            auto* self = const_cast<Iterator*>(this);
            while (self->filterBase_->hasNext()) {
                Value candidate = self->filterBase_->next();
                if (self->filterPred_ && self->filterPred_(candidate)) {
                    self->filterBuffered_ = candidate;
                    self->filterHasBuffered_ = true;
                    return true;
                }
            }
            return false;
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
        } else if (type_ == IteratorType::STRING_ITERATOR) {
            // Return one-character string values
            char ch = string_[currentIndex_++];
            std::string s(1, ch);
            return Value(s);
        } else if (type_ == IteratorType::ENUMERATE_ITERATOR) {
            // Build [index, value]
            Value v = enumerateBase_->next();
            Value idxVal(static_cast<int64_t>(enumerateIndex_++));
            std::vector<Value> pair{idxVal, v};
            size_t listIdx = vmListCreate(pair);
            return Value(static_cast<uint32_t>(listIdx), ValueType::LIST);
        } else if (type_ == IteratorType::ZIP_ITERATOR) {
            std::vector<Value> group;
            group.reserve(zipBases_.size());
            for (auto* it : zipBases_) { group.emplace_back(it->next()); }
            size_t listIdx = vmListCreate(group);
            return Value(static_cast<uint32_t>(listIdx), ValueType::LIST);
        } else if (type_ == IteratorType::MAP_ITERATOR) {
            Value v = mapBase_->next();
            if (mapFunc_) return mapFunc_(v);
            return v;
        } else if (type_ == IteratorType::FILTER_ITERATOR) {
            if (filterHasBuffered_) {
                filterHasBuffered_ = false;
                return filterBuffered_;
            }
            // Advance until predicate passes
            while (filterBase_->hasNext()) {
                Value candidate = filterBase_->next();
                if (!filterPred_ || filterPred_(candidate)) {
                    return candidate;
                }
            }
            return Value();
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
    VM* vm_ = nullptr;
    ListValue* list_ = nullptr;
    DictValue* dict_ = nullptr;
    TupleValue* tuple_ = nullptr;
    SetValue* set_ = nullptr;
    std::string string_;

    // Enumerate wrapper state
    Iterator* enumerateBase_ = nullptr;
    int64_t enumerateIndex_ = 0;

    // Zip wrapper state
    std::vector<Iterator*> zipBases_;

    // Map wrapper state
    Iterator* mapBase_ = nullptr;
    std::function<Value(const Value&)> mapFunc_;

    // Filter wrapper state (lookahead buffer)
    Iterator* filterBase_ = nullptr;
    std::function<bool(const Value&)> filterPred_;
    Value filterBuffered_;
    bool filterHasBuffered_ = false;

    // Helper: create list via VM ListStorage
    size_t vmListCreate(const std::vector<Value>& items) const;
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

    // Create a new string iterator
    size_t createStringIterator(const std::string& str, size_t startIndex = 0) {
        iterators_.push_back(std::make_unique<Iterator>(str, startIndex));
        return iterators_.size() - 1;
    }

    // Create enumerate wrapper iterator
    size_t createEnumerateIterator(VM* vm, Iterator* base, int64_t startIndex = 0) {
        iterators_.push_back(std::make_unique<Iterator>(vm, base, startIndex));
        return iterators_.size() - 1;
    }

    // Create zip wrapper iterator
    size_t createZipIterator(VM* vm, const std::vector<Iterator*>& bases) {
        iterators_.push_back(std::make_unique<Iterator>(vm, bases));
        return iterators_.size() - 1;
    }

    // Create map wrapper iterator
    size_t createMapIterator(VM* vm, Iterator* base, std::function<Value(const Value&)> func) {
        iterators_.push_back(std::make_unique<Iterator>(vm, base, std::move(func)));
        return iterators_.size() - 1;
    }

    // Create filter wrapper iterator
    size_t createFilterIterator(VM* vm, Iterator* base, std::function<bool(const Value&)> pred) {
        iterators_.push_back(std::make_unique<Iterator>(vm, base, std::move(pred)));
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
