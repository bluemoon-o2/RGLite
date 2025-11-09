#include "MemoryManager.h"
#include "ListValue.h"
#include "DictValue.h"
#include "TupleValue.h"
#include "SetValue.h"
#include <iostream>
#include <algorithm>

namespace rglite {

MemoryManager::MemoryManager() : autoGCEnabled_(true), gcThreshold_(1000), listStorage_(nullptr), dictStorage_(nullptr), tupleStorage_(nullptr), setStorage_(nullptr) {
}

MemoryManager::~MemoryManager() {
    // All smart pointers will automatically clean up
}

ListValue* MemoryManager::allocateList() {
    // Don't create objects here, just return nullptr
    // The actual creation should be done by ListStorage
    return nullptr;
}

ListValue* MemoryManager::allocateList(size_t size) {
    // List creation is now handled by ListStorage
    (void)size; // Suppress unused parameter warning
    return nullptr;
}

DictValue* MemoryManager::allocateDict() {
    // Don't create objects here, just return nullptr
    // The actual creation should be done by DictStorage
    return nullptr;
}

TupleValue* MemoryManager::allocateTuple() {
    // Don't create objects here, just return nullptr
    // The actual creation should be done by TupleStorage
    return nullptr;
}

TupleValue* MemoryManager::allocateTuple(size_t size) {
    // Tuple creation is now handled by TupleStorage
    (void)size; // Suppress unused parameter warning
    return nullptr;
}

SetValue* MemoryManager::allocateSet() {
    // Don't create objects here, just return nullptr
    // The actual creation should be done by SetStorage
    return nullptr;
}

void MemoryManager::incrementReference(const Value& value) {
    // Note: In our current implementation, we don't use reference counting
    // This is a placeholder for future implementation
    (void)value; // Suppress unused parameter warning
}

void MemoryManager::decrementReference(const Value& value) {
    // Note: In our current implementation, we don't use reference counting
    // This is a placeholder for future implementation
    (void)value; // Suppress unused parameter warning
}

void MemoryManager::collectGarbage(const std::vector<Value>& roots) {
    // Mark phase
    markPhase(roots);
    
    // Sweep phase
    sweepPhase();
}

size_t MemoryManager::getAllocatedObjectCount() const {
    // If we have storage references, use them
    if (listStorage_ && dictStorage_ && tupleStorage_ && setStorage_) {
        return listStorage_->getCount() + dictStorage_->getCount() + 
               tupleStorage_->getCount() + setStorage_->getCount();
    }
    
    // Fall back to our own tracking (for backward compatibility)
    return lists_.size() + dicts_.size() + tuples_.size() + sets_.size();
}

size_t MemoryManager::getMemoryUsage() const {
    size_t total = 0;
    
    // Calculate memory used by lists
    for (const auto& list : lists_) {
        total += sizeof(ListValue) + list->capacity() * sizeof(Value);
    }
    
    // Calculate memory used by dictionaries
    for (const auto& dict : dicts_) {
        total += sizeof(DictValue) + dict->bucket_count() * sizeof(std::pair<std::string, Value>);
    }
    
    // Calculate memory used by tuples
    for (const auto& tuple : tuples_) {
        total += sizeof(TupleValue) + tuple->size() * sizeof(Value);
    }
    
    // Calculate memory used by sets
    for (const auto& set : sets_) {
        total += sizeof(SetValue) + set->size() * sizeof(Value);
    }
    
    return total;
}

void MemoryManager::setAutoGC(bool enabled) {
    autoGCEnabled_ = enabled;
}

void MemoryManager::setGCThreshold(size_t threshold) {
    gcThreshold_ = threshold;
}

size_t MemoryManager::getGcThreshold() const {
    return gcThreshold_;
}

size_t MemoryManager::getAllocatedLists() const {
    // If we have listStorage reference, use it
    if (listStorage_) {
        return listStorage_->getCount();
    }
    
    // Fall back to our own tracking (for backward compatibility)
    return lists_.size();
}

size_t MemoryManager::getAllocatedDicts() const {
    // If we have dictStorage reference, use it
    if (dictStorage_) {
        return dictStorage_->getCount();
    }
    
    // Fall back to our own tracking (for backward compatibility)
    return dicts_.size();
}

size_t MemoryManager::getAllocatedTuples() const {
    // If we have tupleStorage reference, use it
    if (tupleStorage_) {
        return tupleStorage_->getCount();
    }
    
    // Fall back to our own tracking (for backward compatibility)
    return tuples_.size();
}

size_t MemoryManager::getAllocatedSets() const {
    // If we have setStorage reference, use it
    if (setStorage_) {
        return setStorage_->getCount();
    }
    
    // Fall back to our own tracking (for backward compatibility)
    return sets_.size();
}

void MemoryManager::setListStorage(ListStorage* listStorage) {
    listStorage_ = listStorage;
}

void MemoryManager::setDictStorage(DictStorage* dictStorage) {
    dictStorage_ = dictStorage;
}

void MemoryManager::setTupleStorage(TupleStorage* tupleStorage) {
    tupleStorage_ = tupleStorage;
}

void MemoryManager::setSetStorage(SetStorage* setStorage) {
    setStorage_ = setStorage;
}

void MemoryManager::markPhase(const std::vector<Value>& roots) {
    // Clear all marks first
    clearMarks();
    
    // Mark all objects reachable from roots
    for (const auto& root : roots) {
        markValue(root);
    }
}

void MemoryManager::markValue(const Value& value) {
    if (value.isList()) {
        uint32_t index = value.asIndex();
        if (index < lists_.size()) {
            markList(lists_[index].get());
        }
    } else if (value.isDict()) {
        uint32_t index = value.asIndex();
        if (index < dicts_.size()) {
            markDict(dicts_[index].get());
        }
    } else if (value.isTuple()) {
        uint32_t index = value.asIndex();
        if (index < tuples_.size()) {
            markTuple(tuples_[index].get());
        }
    } else if (value.isSet()) {
        uint32_t index = value.asIndex();
        if (index < sets_.size()) {
            markSet(sets_[index].get());
        }
    }
}

void MemoryManager::sweepPhase() {
    // Remove unmarked lists
    lists_.erase(
        std::remove_if(lists_.begin(), lists_.end(),
            [this](const std::unique_ptr<ListValue>& list) {
                return !isMarked(list.get());
            }),
        lists_.end()
    );
    
    // Remove unmarked dictionaries
    dicts_.erase(
        std::remove_if(dicts_.begin(), dicts_.end(),
            [this](const std::unique_ptr<DictValue>& dict) {
                return !isMarked(dict.get());
            }),
        dicts_.end()
    );
    
    // Remove unmarked tuples
    tuples_.erase(
        std::remove_if(tuples_.begin(), tuples_.end(),
            [this](const std::unique_ptr<TupleValue>& tuple) {
                return !isMarked(tuple.get());
            }),
        tuples_.end()
    );
    
    // Remove unmarked sets
    sets_.erase(
        std::remove_if(sets_.begin(), sets_.end(),
            [this](const std::unique_ptr<SetValue>& set) {
                return !isMarked(set.get());
            }),
        sets_.end()
    );
}

void MemoryManager::markList(ListValue* list) {
    if (!list || isMarked(list)) {
        return;
    }
    
    // Mark this list
    setMark(list);
    
    // Mark all elements in the list
    for (size_t i = 0; i < list->size(); ++i) {
        markValue(list->get(i));
    }
}

void MemoryManager::markDict(DictValue* dict) {
    if (!dict || isMarked(dict)) {
        return;
    }
    
    // Mark this dictionary
    setMark(dict);
    
    // Mark all values in the dictionary
    // Use the public API to iterate through items
    const auto items = dict->getItems();
    for (const auto& kv : items) {
        // Keys are strings and don't require marking; mark values only
        markValue(kv.second);
    }
}

void MemoryManager::markTuple(TupleValue* tuple) {
    if (!tuple || isMarked(tuple)) {
        return;
    }
    
    // Mark this tuple
    setMark(tuple);
    
    // Mark all elements in the tuple
    for (size_t i = 0; i < tuple->size(); ++i) {
        markValue(tuple->get(i));
    }
}

void MemoryManager::markSet(SetValue* set) {
    if (!set || isMarked(set)) {
        return;
    }
    
    // Mark this set
    setMark(set);
    
    // Mark all elements in the set
    // Use the public API to get items and mark each
    const auto items = set->getItems();
    for (const auto& v : items) {
        markValue(v);
    }
}

bool MemoryManager::isMarked(const void* obj) const {
    return markedObjects_.find(obj) != markedObjects_.end();
}

void MemoryManager::setMark(const void* obj) {
    markedObjects_.insert(obj);
}

void MemoryManager::clearMarks() {
    markedObjects_.clear();
}

void MemoryManager::freeUnmarkedObjects() {
    // This is handled in sweepPhase
}

} // namespace rglite
