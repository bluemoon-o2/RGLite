#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <cstddef>

namespace rglite {

// Forward declarations
class Value;
class ListValue;
class DictValue;
class TupleValue;
class SetValue;
class ListStorage;
class DictStorage;
class TupleStorage;
class SetStorage;

/**
 * @brief Memory manager for handling garbage collection and memory allocation
 * 
 * This class implements a reference counting and mark-and-sweep garbage collector
 * to manage memory for heap-allocated objects like lists and dictionaries.
 */
class MemoryManager {
public:
    /**
     * @brief Constructor for MemoryManager
     */
    MemoryManager();
    
    /**
     * @brief Destructor for MemoryManager
     */
    ~MemoryManager();
    
    /**
     * @brief Allocate a new ListValue object
     * @return Pointer to the allocated ListValue
     */
    ListValue* allocateList();
    
    /**
     * @brief Allocate a new ListValue object with initial size
     * @param size Initial size of the list
     * @return Pointer to the allocated ListValue
     */
    ListValue* allocateList(size_t size);
    
    /**
     * @brief Allocate a new DictValue object
     * @return Pointer to the allocated DictValue
     */
    DictValue* allocateDict();
    
    /**
     * @brief Allocate a new TupleValue object
     * @return Pointer to the allocated TupleValue
     */
    TupleValue* allocateTuple();
    
    /**
     * @brief Allocate a new TupleValue object with initial size
     * @param size Initial size of the tuple
     * @return Pointer to the allocated TupleValue
     */
    TupleValue* allocateTuple(size_t size);
    
    /**
     * @brief Allocate a new SetValue object
     * @return Pointer to the allocated SetValue
     */
    SetValue* allocateSet();
    
    /**
     * @brief Increment reference count for a value
     * @param value The value to increment reference count for
     */
    void incrementReference(const Value& value);
    
    /**
     * @brief Decrement reference count for a value
     * @param value The value to decrement reference count for
     */
    void decrementReference(const Value& value);
    
    /**
     * @brief Trigger garbage collection
     * @param roots Vector of root values to start marking from
     */
    void collectGarbage(const std::vector<Value>& roots);
    
    /**
     * @brief Get the current number of allocated objects
     * @return Number of allocated objects
     */
    size_t getAllocatedObjectCount() const;
    
    /**
     * @brief Get the current memory usage in bytes
     * @return Memory usage in bytes
     */
    size_t getMemoryUsage() const;
    
    /**
     * @brief Enable or disable automatic garbage collection
     * @param enabled Whether to enable automatic GC
     */
    void setAutoGC(bool enabled);
    
    /**
     * @brief Set the threshold for automatic garbage collection
     * @param threshold Number of objects that triggers GC
     */
    void setGCThreshold(size_t threshold);
    
    /**
     * @brief Get the current threshold for automatic garbage collection
     * @return Number of objects that triggers GC
     */
    size_t getGcThreshold() const;
    
    /**
     * @brief Get the current number of allocated lists
     * @return Number of allocated lists
     */
    size_t getAllocatedLists() const;
    
    /**
     * @brief Get the current number of allocated dictionaries
     * @return Number of allocated dictionaries
     */
    size_t getAllocatedDicts() const;
    
    /**
     * @brief Get the current number of allocated tuples
     * @return Number of allocated tuples
     */
    size_t getAllocatedTuples() const;
    
    /**
     * @brief Get the current number of allocated sets
     * @return Number of allocated sets
     */
    size_t getAllocatedSets() const;
    
    /**
     * @brief Set the ListStorage instance
     * @param listStorage Pointer to the ListStorage instance
     */
    void setListStorage(ListStorage* listStorage);
    
    /**
     * @brief Set the DictStorage instance
     * @param dictStorage Pointer to the DictStorage instance
     */
    void setDictStorage(DictStorage* dictStorage);
    
    /**
     * @brief Set the TupleStorage instance
     * @param tupleStorage Pointer to the TupleStorage instance
     */
    void setTupleStorage(TupleStorage* tupleStorage);
    
    /**
     * @brief Set the SetStorage instance
     * @param setStorage Pointer to the SetStorage instance
     */
    void setSetStorage(SetStorage* setStorage);

private:
    // Storage references
    ListStorage* listStorage_;
    DictStorage* dictStorage_;
    TupleStorage* tupleStorage_;
    SetStorage* setStorage_;
    
    // Memory tracking (kept for compatibility but not used)
    std::vector<std::unique_ptr<ListValue>> lists_;
    std::vector<std::unique_ptr<DictValue>> dicts_;
    std::vector<std::unique_ptr<TupleValue>> tuples_;
    std::vector<std::unique_ptr<SetValue>> sets_;
    
    // Reference counting
    std::unordered_map<const void*, size_t> refCounts_;
    
    // GC settings
    bool autoGCEnabled_;
    size_t gcThreshold_;
    
    // Mark tracking for garbage collection
    std::unordered_set<const void*> markedObjects_;
    
    // Mark phase for mark-and-sweep GC
    void markPhase(const std::vector<Value>& roots);
    
    // Mark a single value
    void markValue(const Value& value);
    
    // Sweep phase for mark-and-sweep GC
    void sweepPhase();
    
    // Mark a ListValue
    void markList(ListValue* list);
    
    // Mark a DictValue
    void markDict(DictValue* dict);
    
    // Mark a TupleValue
    void markTuple(TupleValue* tuple);
    
    // Mark a SetValue
    void markSet(SetValue* set);
    
    // Check if an object is marked
    bool isMarked(const void* obj) const;
    
    // Set mark on an object
    void setMark(const void* obj);
    
    // Clear marks
    void clearMarks();
    
    // Free unmarked objects
    void freeUnmarkedObjects();
};

} // namespace rglite

#endif // MEMORY_MANAGER_H