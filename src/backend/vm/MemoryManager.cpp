#include "MemoryManager.h"
#include "Value.h"
#include "ListValue.h"
#include "DictValue.h"
#include <iostream>
#include <algorithm>

namespace RGLite {

MemoryManager::MemoryManager() : autoGCEnabled_(true), gcThreshold_(1000), listStorage_(nullptr), dictStorage_(nullptr) {
}

MemoryManager::~MemoryManager() {
    // All smart pointers will automatically clean up
}

ListValue* MemoryManager::allocateList() {
    auto list = std::make_unique<ListValue>();
    ListValue* ptr = list.get();
    lists_.push_back(std::move(list));
    
    // Check if we should trigger GC
    if (autoGCEnabled_ && getAllocatedObjectCount() > gcThreshold_) {
        // Note: In a real implementation, we would need access to the VM's roots
        // For now, we'll skip automatic GC in this method
    }
    
    return ptr;
}

DictValue* MemoryManager::allocateDict() {
    auto dict = std::make_unique<DictValue>();
    DictValue* ptr = dict.get();
    dicts_.push_back(std::move(dict));
    
    // Check if we should trigger GC
    if (autoGCEnabled_ && getAllocatedObjectCount() > gcThreshold_) {
        // Note: In a real implementation, we would need access to the VM's roots
        // For now, we'll skip automatic GC in this method
    }
    
    return ptr;
}

void MemoryManager::incrementReference(const Value& value) {
    // Note: In our current implementation, we don't use reference counting
    // This is a placeholder for future implementation
}

void MemoryManager::decrementReference(const Value& value) {
    // Note: In our current implementation, we don't use reference counting
    // This is a placeholder for future implementation
}

void MemoryManager::collectGarbage(const std::vector<Value>& roots) {
    // Mark phase
    markPhase(roots);
    
    // Sweep phase
    sweepPhase();
}

size_t MemoryManager::getAllocatedObjectCount() const {
    return lists_.size() + dicts_.size();
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
    
    return total;
}

size_t MemoryManager::getAllocatedLists() const {
    // If listStorage_ is available, use its count
    if (listStorage_) {
        return listStorage_->getAllocatedLists();
    }
    
    // Otherwise, use the internal count (for backward compatibility)
    return lists_.size();
}

size_t MemoryManager::getAllocatedDicts() const {
    // If dictStorage_ is available, use its count
    if (dictStorage_) {
        return dictStorage_->getAllocatedDicts();
    }
    
    // Otherwise, use the internal count (for backward compatibility)
    return dicts_.size();
}

void MemoryManager::setListStorage(ListStorage* listStorage) {
    listStorage_ = listStorage;
}

void MemoryManager::setDictStorage(DictStorage* dictStorage) {
    dictStorage_ = dictStorage;
}

void MemoryManager::setAutoGC(bool enabled) {
    autoGCEnabled_ = enabled;
}

void MemoryManager::setGCThreshold(size_t threshold) {
    gcThreshold_ = threshold;
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
    
    // Mark all keys and values in the dictionary
    // Use getItems() to iterate through all key-value pairs
    auto items = dict->getItems();
    for (const auto& pair : items) {
        // Keys are strings, which are not heap-allocated in our implementation
        markValue(pair.second); // Mark the value
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

} // namespace RGLite