// BuiltinFunctions.h - Declaration of builtin functions for RGLite
// This file contains the declarations of all builtin functions available in RGLite

#ifndef RGLITE_BUILTIN_FUNCTIONS_H
#define RGLITE_BUILTIN_FUNCTIONS_H

#include "Bytecode.h"
#include <vector>

namespace rglite {

class VM; // Forward declaration

// Type checking functions
Value nativeType(VM& vm, const std::vector<Value>& args);
Value nativeIsNil(VM& vm, const std::vector<Value>& args);
Value nativeIsBoolean(VM& vm, const std::vector<Value>& args);
Value nativeIsInteger(VM& vm, const std::vector<Value>& args);
Value nativeIsFloat(VM& vm, const std::vector<Value>& args);
Value nativeIsNumber(VM& vm, const std::vector<Value>& args);
Value nativeIsString(VM& vm, const std::vector<Value>& args);
Value nativeIsList(VM& vm, const std::vector<Value>& args);
Value nativeIsDict(VM& vm, const std::vector<Value>& args);
Value nativeIsFunction(VM& vm, const std::vector<Value>& args);

// Math functions
Value nativeAbs(VM& vm, const std::vector<Value>& args);
Value nativeMin(VM& vm, const std::vector<Value>& args);
Value nativeMax(VM& vm, const std::vector<Value>& args);
Value nativeSum(VM& vm, const std::vector<Value>& args);

// Conversion functions
Value nativeInt(VM& vm, const std::vector<Value>& args);

// String functions
Value nativeStr(VM& vm, const std::vector<Value>& args);
Value nativeSubstr(VM& vm, const std::vector<Value>& args);

// Iteration helpers
Value nativeIter(VM& vm, const std::vector<Value>& args);
Value nativeNext(VM& vm, const std::vector<Value>& args);
Value nativeEnumerate(VM& vm, const std::vector<Value>& args);
Value nativeZip(VM& vm, const std::vector<Value>& args);
Value nativeMap(VM& vm, const std::vector<Value>& args);
Value nativeFilter(VM& vm, const std::vector<Value>& args);

// List functions
Value nativeAppend(VM& vm, const std::vector<Value>& args);
Value nativeRemove(VM& vm, const std::vector<Value>& args);
Value nativeExtend(VM& vm, const std::vector<Value>& args);
Value nativeInsert(VM& vm, const std::vector<Value>& args);
Value nativePop(VM& vm, const std::vector<Value>& args);
Value nativeClear(VM& vm, const std::vector<Value>& args);
Value nativeSort(VM& vm, const std::vector<Value>& args);
Value nativeReverse(VM& vm, const std::vector<Value>& args);
Value nativeSorted(VM& vm, const std::vector<Value>& args);
Value nativeReversed(VM& vm, const std::vector<Value>& args);
Value nativeCount(VM& vm, const std::vector<Value>& args);
Value nativeIndex(VM& vm, const std::vector<Value>& args);
Value nativeListCopy(VM& vm, const std::vector<Value>& args);

// Dict functions
Value nativeKeys(VM& vm, const std::vector<Value>& args);
Value nativeValues(VM& vm, const std::vector<Value>& args);
Value nativeContains(VM& vm, const std::vector<Value>& args);
Value nativeUpdate(VM& vm, const std::vector<Value>& args);
Value nativeGet(VM& vm, const std::vector<Value>& args);
Value nativeCopy(VM& vm, const std::vector<Value>& args);
Value nativeFromKeys(VM& vm, const std::vector<Value>& args);
Value nativeItems(VM& vm, const std::vector<Value>& args);
Value nativeDictPop(VM& vm, const std::vector<Value>& args);
Value nativePopItem(VM& vm, const std::vector<Value>& args);
Value nativeSetDefault(VM& vm, const std::vector<Value>& args);

// I/O functions
Value nativePrint(VM& vm, const std::vector<Value>& args);
Value nativeLen(VM& vm, const std::vector<Value>& args);
Value nativeRange(VM& vm, const std::vector<Value>& args);

// Import function
Value nativeImport(VM& vm, const std::vector<Value>& args);
// Import-all helper
Value nativeImportAll(VM& vm, const std::vector<Value>& args);

// Logical aggregation functions
Value nativeAny(VM& vm, const std::vector<Value>& args);
Value nativeAll(VM& vm, const std::vector<Value>& args);

// Register all builtin functions with the VM
void registerBuiltinFunctions(VM& vm);

} // namespace rglite

#endif // RGLITE_BUILTIN_FUNCTIONS_H
