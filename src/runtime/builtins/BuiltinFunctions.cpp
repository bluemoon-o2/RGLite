// BuiltinFunctions.cpp - Implementation of builtin functions for RGLite
// This file contains the implementations of all builtin functions available in RGLite

#include "BuiltinFunctions.h"
#include "VM.h"
#include "Exception.h"
#include "ListValue.h"
#include "DictValue.h"
#include "TupleValue.h"
#include "SetValue.h"
#include "StringUtils.h"
#include "Lexer.h"
#include "Parser.h"
#include "SemanticAnalyzer.h"
#include "CodeGenerator.h"
#include "ErrorHandler.h"
#include <filesystem>
#include <fstream>
#include <unordered_set>
#include <iostream>
#include <cmath>
#include <algorithm>

namespace rglite {

// Helper for truthiness evaluation
static bool isTruthy(VM& vm, const Value& v) {
    if (v.isNil()) return false;
    if (v.isBoolean()) return v.asBoolean();
    if (v.isInteger()) return v.asInteger() != 0;
    if (v.isFloat()) return v.asFloat() != 0.0;
    if (v.isString()) return !v.asString().empty();
    if (v.isList()) {
        const ListValue* list = vm.getListStorage().getList(v.asIndex());
        if (!list) {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid list reference"));
            return false;
        }
        return list->size() != 0;
    }
    if (v.isDict()) {
        const DictValue* dict = vm.getDictStorage().getDict(v.asIndex());
        if (!dict) {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid dictionary reference"));
            return false;
        }
        return dict->size() != 0;
    }
    if (v.isTuple()) {
        const TupleValue* tup = vm.getTupleStorage().getTuple(v.asIndex());
        if (!tup) {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid tuple reference"));
            return false;
        }
        return tup->size() != 0;
    }
    if (v.isSet()) {
        const SetValue* setv = vm.getSetStorage().getSet(v.asIndex());
        if (!setv) {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid set reference"));
            return false;
        }
        return setv->size() != 0;
    }
    return true;
}

// Helper function to determine type order for sorting
int getTypeOrder(const Value& value) {
    if (value.isInteger() || value.isFloat()) return 0;
    if (value.isString()) return 1;
    if (value.isBoolean()) return 2;
    if (value.isNil()) return 3;
    if (value.isFunction()) return 4;
    if (value.isList()) return 5;
    if (value.isDict()) return 6;
    return 7; // Unknown type
}

// Iteration helpers
Value nativeIter(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 1) {
        vm.throwException(ExceptionBuilder::typeError("1", std::to_string(args.size())));
        return Value();
    }
    const Value& obj = args[0];
    if (obj.isList()) {
        ListValue* list = vm.getListStorage().getList(obj.asIndex());
        if (!list) { vm.throwException(ExceptionBuilder::runtimeError("Invalid list reference")); return Value(); }
        size_t iterIndex = vm.getIteratorStorage().createListIterator(list);
        return Value(static_cast<uint32_t>(iterIndex), ValueType::ITERATOR);
    } else if (obj.isDict()) {
        DictValue* dict = vm.getDictStorage().getDict(obj.asIndex());
        if (!dict) { vm.throwException(ExceptionBuilder::runtimeError("Invalid dictionary reference")); return Value(); }
        size_t iterIndex = vm.getIteratorStorage().createDictIterator(dict, IteratorType::DICT_KEY_ITERATOR);
        return Value(static_cast<uint32_t>(iterIndex), ValueType::ITERATOR);
    } else if (obj.isTuple()) {
        TupleValue* tup = vm.getTupleStorage().getTuple(obj.asIndex());
        if (!tup) { vm.throwException(ExceptionBuilder::runtimeError("Invalid tuple reference")); return Value(); }
        size_t iterIndex = vm.getIteratorStorage().createTupleIterator(tup);
        return Value(static_cast<uint32_t>(iterIndex), ValueType::ITERATOR);
    } else if (obj.isSet()) {
        SetValue* setv = vm.getSetStorage().getSet(obj.asIndex());
        if (!setv) { vm.throwException(ExceptionBuilder::runtimeError("Invalid set reference")); return Value(); }
        size_t iterIndex = vm.getIteratorStorage().createSetIterator(setv);
        return Value(static_cast<uint32_t>(iterIndex), ValueType::ITERATOR);
    } else if (obj.isString()) {
        size_t iterIndex = vm.getIteratorStorage().createStringIterator(obj.asString());
        return Value(static_cast<uint32_t>(iterIndex), ValueType::ITERATOR);
    }
    vm.throwException(ExceptionBuilder::typeError("iterable", "non-iterable type"));
    return Value();
}

Value nativeNext(VM& vm, const std::vector<Value>& args) {
    if (args.size() < 1 || args.size() > 2) {
        vm.throwException(ExceptionBuilder::typeError("1 or 2", std::to_string(args.size())));
        return Value();
    }
    const Value& itVal = args[0];
    if (!itVal.isIterator()) {
        vm.throwException(ExceptionBuilder::typeError("iterator", "non-iterator type"));
        return Value();
    }
    Iterator* it = vm.getIteratorStorage().getIterator(itVal.asIndex());
    if (!it) { vm.throwException(ExceptionBuilder::runtimeError("Invalid iterator reference")); return Value(); }
    if (!it->hasNext()) {
        if (args.size() == 2) return args[1];
        vm.throwException(ExceptionBuilder::runtimeError("StopIteration"));
        return Value();
    }
    return it->next();
}

Value nativeEnumerate(VM& vm, const std::vector<Value>& args) {
    if (args.size() < 1 || args.size() > 2) {
        vm.throwException(ExceptionBuilder::typeError("1 or 2", std::to_string(args.size())));
        return Value();
    }
    int64_t start = 0;
    if (args.size() == 2) {
        if (!args[1].isInteger()) {
            vm.throwException(ExceptionBuilder::typeError("integer", "non-integer start index"));
            return Value();
        }
        start = args[1].asInteger();
    }

    // Create base iterator and wrap as enumerate iterator (lazy)
    Value iterVal = nativeIter(vm, {args[0]});
    if (vm.hasException()) return Value();
    Iterator* base = vm.getIteratorStorage().getIterator(iterVal.asIndex());
    if (!base) { vm.throwException(ExceptionBuilder::runtimeError("Invalid iterator reference")); return Value(); }
    size_t enumIdx = vm.getIteratorStorage().createEnumerateIterator(&vm, base, start);
    return Value(static_cast<uint32_t>(enumIdx), ValueType::ITERATOR);
}

Value nativeZip(VM& vm, const std::vector<Value>& args) {
    if (args.size() < 2) {
        vm.throwException(ExceptionBuilder::typeError(">=2", std::to_string(args.size())));
        return Value();
    }
    // Build base iterators and wrap as zip iterator (lazy)
    std::vector<Iterator*> bases;
    bases.reserve(args.size());
    for (const Value& a : args) {
        Value iterVal = nativeIter(vm, {a});
        if (vm.hasException()) return Value();
        Iterator* it = vm.getIteratorStorage().getIterator(iterVal.asIndex());
        if (!it) { vm.throwException(ExceptionBuilder::runtimeError("Invalid iterator reference")); return Value(); }
        bases.push_back(it);
    }
    size_t zipIdx = vm.getIteratorStorage().createZipIterator(&vm, bases);
    return Value(static_cast<uint32_t>(zipIdx), ValueType::ITERATOR);
}

// Helper to call a subset of builtin functions by name with one argument
static Value callBuiltinOneArg(VM& vm, const std::string& name, const Value& arg) {
    std::vector<Value> single{arg};
    if (name == "str") return nativeStr(vm, single);
    if (name == "int") return nativeInt(vm, single);
    if (name == "abs") return nativeAbs(vm, single);
    if (name == "isnumber") return nativeIsNumber(vm, single);
    if (name == "isstring") return nativeIsString(vm, single);
    if (name == "isboolean") return nativeIsBoolean(vm, single);
    if (name == "isinteger") return nativeIsInteger(vm, single);
    if (name == "isfloat") return nativeIsFloat(vm, single);
    vm.throwException(ExceptionBuilder::runtimeError(std::string("Unsupported builtin in map/filter: ") + name));
    return Value();
}

Value nativeMap(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 2) {
        vm.throwException(ExceptionBuilder::typeError("2", std::to_string(args.size())));
        return Value();
    }
    // Accept both orders: map(func, iterable) or map(iterable, func)
    std::string funcName;
    Value iterableArg;
    auto isFunc = [](const Value& v) { return v.isNativeFunction() || v.isString(); };
    if (isFunc(args[0])) {
        funcName = args[0].isNativeFunction() ? args[0].asNativeFunctionName() : args[0].asString();
        iterableArg = args[1];
    } else if (isFunc(args[1])) {
        funcName = args[1].isNativeFunction() ? args[1].asNativeFunctionName() : args[1].asString();
        iterableArg = args[0];
    } else {
        vm.throwException(ExceptionBuilder::typeError("builtin function name", "non-native function"));
        return Value();
    }

    Value iterVal = nativeIter(vm, {iterableArg});
    if (vm.hasException()) return Value();
    Iterator* base = vm.getIteratorStorage().getIterator(iterVal.asIndex());
    if (!base) { vm.throwException(ExceptionBuilder::runtimeError("Invalid iterator reference")); return Value(); }
    auto mapFn = [&vm, funcName](const Value& v) -> Value {
        return callBuiltinOneArg(vm, funcName, v);
    };
    size_t mapIdx = vm.getIteratorStorage().createMapIterator(&vm, base, mapFn);
    return Value(static_cast<uint32_t>(mapIdx), ValueType::ITERATOR);
}

Value nativeFilter(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 2) {
        vm.throwException(ExceptionBuilder::typeError("2", std::to_string(args.size())));
        return Value();
    }
    // Accept both orders: filter(func, iterable) or filter(iterable, func)
    std::string funcName;
    Value iterableArg;
    auto isFunc = [](const Value& v) { return v.isNativeFunction() || v.isString(); };
    if (isFunc(args[0])) {
        funcName = args[0].isNativeFunction() ? args[0].asNativeFunctionName() : args[0].asString();
        iterableArg = args[1];
    } else if (isFunc(args[1])) {
        funcName = args[1].isNativeFunction() ? args[1].asNativeFunctionName() : args[1].asString();
        iterableArg = args[0];
    } else {
        vm.throwException(ExceptionBuilder::typeError("builtin function name", "non-native function"));
        return Value();
    }
    Value iterVal = nativeIter(vm, {iterableArg});
    if (vm.hasException()) return Value();
    Iterator* base = vm.getIteratorStorage().getIterator(iterVal.asIndex());
    if (!base) { vm.throwException(ExceptionBuilder::runtimeError("Invalid iterator reference")); return Value(); }
    auto predFn = [&vm, funcName](const Value& v) -> bool {
        Value res = callBuiltinOneArg(vm, funcName, v);
        if (vm.hasException()) return false;
        return isTruthy(vm, res);
    };
    size_t filIdx = vm.getIteratorStorage().createFilterIterator(&vm, base, predFn);
    return Value(static_cast<uint32_t>(filIdx), ValueType::ITERATOR);
}
// I/O functions
Value nativePrint(VM& vm, const std::vector<Value>& args) {
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) std::cout << " ";
        
        const Value& arg = args[i];
        
        // Special handling for list values
        if (arg.isList()) {
            uint32_t listIndex = arg.asIndex();
            const ListValue* list = vm.getListStorage().getList(listIndex);
            if (list) {
                // Detect range-like integer arithmetic progression and print as Python's range(...)
                const auto& items = list->getItems();
                bool allInts = true;
                for (const auto& it : items) {
                    if (!it.isInteger()) { allInts = false; break; }
                }
                if (allInts && !items.empty()) {
                    int64_t start = items.front().asInteger();
                    int64_t step = 1;
                    if (items.size() >= 2) {
                        step = items[1].asInteger() - items[0].asInteger();
                        if (step == 0) {
                            std::cout << list->toString();
                            goto after_print_list;
                        }
                        bool constantStep = true;
                        for (size_t k = 2; k < items.size(); ++k) {
                            int64_t diff = items[k].asInteger() - items[k - 1].asInteger();
                            if (diff != step) { constantStep = false; break; }
                        }
                        if (!constantStep) {
                            std::cout << list->toString();
                            goto after_print_list;
                        }
                    }
                    int64_t stop = items.back().asInteger() + step;
                    if (start == 0 && step == 1) {
                        std::cout << "range(" << stop << ")";
                    } else if (step == 1) {
                        std::cout << "range(" << start << ", " << stop << ")";
                    } else {
                        std::cout << "range(" << start << ", " << stop << ", " << step << ")";
                    }
                } else {
                    std::cout << list->toString();
                }
                after_print_list: ;
            } else {
                std::cout << "[invalid list]";
            }
        } else if (arg.isDict()) {
            uint32_t dictIndex = arg.asIndex();
            const DictValue* dict = vm.getDictStorage().getDict(dictIndex);
            if (dict) {
                std::cout << dict->toString();
            } else {
                std::cout << "{invalid dict}";
            }
        } else {
            std::cout << valueToString(arg);
        }
    }
    std::cout << std::endl;
    return Value(); // Return nil
}

Value nativeLen(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 1) {
        vm.throwException(ExceptionBuilder::typeError("1", std::to_string(args.size())));
        return Value();
    }
    
    const Value& arg = args[0];
    if (arg.isString()) {
        return Value(static_cast<int64_t>(arg.asString().length()));
    } else if (arg.isList()) {
        // Get the list from storage
        uint32_t listIndex = arg.asIndex();
        const ListValue* list = vm.getListStorage().getList(listIndex);
        if (list) {
            return Value(static_cast<int64_t>(list->size()));
        } else {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid list reference"));
            return Value();
        }
    } else if (arg.isDict()) {
        // Get the dictionary from storage
        uint32_t dictIndex = arg.asIndex();
        const DictValue* dict = vm.getDictStorage().getDict(dictIndex);
        if (dict) {
            return Value(static_cast<int64_t>(dict->size()));
        } else {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid dictionary reference"));
            return Value();
        }
    } else if (arg.isTuple()) {
        uint32_t tupIndex = arg.asIndex();
        const TupleValue* tup = vm.getTupleStorage().getTuple(tupIndex);
        if (tup) {
            return Value(static_cast<int64_t>(tup->size()));
        } else {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid tuple reference"));
            return Value();
        }
    } else if (arg.isSet()) {
        uint32_t setIndex = arg.asIndex();
        const SetValue* setv = vm.getSetStorage().getSet(setIndex);
        if (setv) {
            return Value(static_cast<int64_t>(setv->size()));
        } else {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid set reference"));
            return Value();
        }
    } else {
        vm.throwException(ExceptionBuilder::typeError("string, list, dict, tuple, or set", "other"));
        return Value();
    }
}

// Type checking functions
Value nativeType(VM& /*vm*/, const std::vector<Value>& args) {
    if (args.size() != 1) {
        return Value(); // Return nil for invalid arguments
    }
    
    const Value& arg = args[0];
    if (arg.isNil()) {
        return Value("nil");
    } else if (arg.isBoolean()) {
        return Value("boolean");
    } else if (arg.isInteger()) {
        return Value("integer");
    } else if (arg.isFloat()) {
        return Value("float");
    } else if (arg.isString()) {
        return Value("string");
    } else if (arg.isList()) {
        return Value("list");
    } else if (arg.isDict()) {
        return Value("dict");
    } else if (arg.isFunction()) {
        return Value("function");
    } else {
        return Value("unknown");
    }
}

Value nativeIsNil(VM& /*vm*/, const std::vector<Value>& args) {
    if (args.size() != 1) {
        return Value(false);
    }
    return Value(args[0].isNil());
}

Value nativeIsBoolean(VM& /*vm*/, const std::vector<Value>& args) {
    if (args.size() != 1) {
        return Value(false);
    }
    return Value(args[0].isBoolean());
}

// range([start], stop[, step]) -> list of integers
// Python-compatible semantics: stop excluded; step default 1; negative step supported; zero step -> ValueError
Value nativeRange(VM& vm, const std::vector<Value>& args) {
    if (args.size() < 1 || args.size() > 3) {
        vm.throwException(ExceptionBuilder::typeError("1 to 3", std::to_string(args.size())));
        return Value();
    }

    // Parse arguments
    int64_t start = 0;
    int64_t stop = 0;
    int64_t step = 1;

    if (args.size() == 1) {
        if (!args[0].isInteger()) {
            vm.throwException(ExceptionBuilder::typeError("integer", "non-integer"));
            return Value();
        }
        stop = args[0].asInteger();
    } else if (args.size() == 2) {
        if (!args[0].isInteger() || !args[1].isInteger()) {
            vm.throwException(ExceptionBuilder::typeError("integer", "non-integer"));
            return Value();
        }
        start = args[0].asInteger();
        stop = args[1].asInteger();
    } else { // 3 args
        if (!args[0].isInteger() || !args[1].isInteger() || !args[2].isInteger()) {
            vm.throwException(ExceptionBuilder::typeError("integer", "non-integer"));
            return Value();
        }
        start = args[0].asInteger();
        stop = args[1].asInteger();
        step = args[2].asInteger();
    }

    if (step == 0) {
        vm.throwException(ExceptionBuilder::valueError("range() arg 3 must not be zero"));
        return Value();
    }

    std::vector<Value> items;
    // Compute sequence with Python semantics: stop excluded
    if (step > 0) {
        for (int64_t i = start; i < stop; i += step) {
            items.emplace_back(i);
        }
    } else { // step < 0
        for (int64_t i = start; i > stop; i += step) {
            items.emplace_back(i);
        }
    }

    size_t listIndex = vm.getListStorage().createList(items);
    return Value(static_cast<uint32_t>(listIndex), ValueType::LIST);
}

Value nativeIsInteger(VM& /*vm*/, const std::vector<Value>& args) {
    if (args.size() != 1) {
        return Value(false);
    }
    return Value(args[0].isInteger());
}

Value nativeIsFloat(VM& /*vm*/, const std::vector<Value>& args) {
    if (args.size() != 1) {
        return Value(false);
    }
    return Value(args[0].isFloat());
}

Value nativeIsNumber(VM& /*vm*/, const std::vector<Value>& args) {
    if (args.size() != 1) {
        return Value(false);
    }
    return Value(args[0].isInteger() || args[0].isFloat());
}

Value nativeIsString(VM& /*vm*/, const std::vector<Value>& args) {
    if (args.size() != 1) {
        return Value(false);
    }
    return Value(args[0].isString());
}

Value nativeIsList(VM& /*vm*/, const std::vector<Value>& args) {
    if (args.size() != 1) {
        return Value(false);
    }
    return Value(args[0].isList());
}

Value nativeIsDict(VM& /*vm*/, const std::vector<Value>& args) {
    if (args.size() != 1) {
        return Value(false);
    }
    return Value(args[0].isDict());
}

Value nativeIsFunction(VM& /*vm*/, const std::vector<Value>& args) {
    if (args.size() != 1) {
        return Value(false);
    }
    return Value(args[0].isFunction());
}

// Math functions
Value nativeAbs(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 1) {
        vm.throwException(ExceptionBuilder::typeError("1", std::to_string(args.size())));
        return Value();
    }
    
    const Value& arg = args[0];
    if (arg.isInteger()) {
        int64_t val = arg.asInteger();
        return Value(val < 0 ? -val : val);
    } else if (arg.isFloat()) {
        double val = arg.asFloat();
        return Value(val < 0.0 ? -val : val);
    } else {
        vm.throwException(ExceptionBuilder::typeError("number", "non-number"));
        return Value();
    }
}

Value nativeMin(VM& vm, const std::vector<Value>& args) {
    if (args.size() < 2) {
        vm.throwException(ExceptionBuilder::typeError("at least 2", std::to_string(args.size())));
        return Value();
    }
    
    // Check if all arguments are numbers
    for (const auto& arg : args) {
        if (!arg.isInteger() && !arg.isFloat()) {
            vm.throwException(ExceptionBuilder::typeError("number", "non-number"));
            return Value();
        }
    }
    
    Value minVal = args[0];
    for (size_t i = 1; i < args.size(); ++i) {
        const Value& arg = args[i];
        
        // Compare values
        bool less = false;
        if (minVal.isInteger() && arg.isInteger()) {
            less = arg.asInteger() < minVal.asInteger();
        } else {
            // Convert to float for comparison
            double minFloat = minVal.isInteger() ? static_cast<double>(minVal.asInteger()) : minVal.asFloat();
            double argFloat = arg.isInteger() ? static_cast<double>(arg.asInteger()) : arg.asFloat();
            less = argFloat < minFloat;
        }
        
        if (less) {
            minVal = arg;
        }
    }
    
    return minVal;
}

Value nativeMax(VM& vm, const std::vector<Value>& args) {
    if (args.size() < 2) {
        vm.throwException(ExceptionBuilder::typeError("at least 2", std::to_string(args.size())));
        return Value();
    }
    
    // Check if all arguments are numbers
    for (const auto& arg : args) {
        if (!arg.isInteger() && !arg.isFloat()) {
            vm.throwException(ExceptionBuilder::typeError("number", "non-number"));
            return Value();
        }
    }
    
    Value maxVal = args[0];
    for (size_t i = 1; i < args.size(); ++i) {
        const Value& arg = args[i];
        
        // Compare values
        bool greater = false;
        if (maxVal.isInteger() && arg.isInteger()) {
            greater = arg.asInteger() > maxVal.asInteger();
        } else {
            // Convert to float for comparison
            double maxFloat = maxVal.isInteger() ? static_cast<double>(maxVal.asInteger()) : maxVal.asFloat();
            double argFloat = arg.isInteger() ? static_cast<double>(arg.asInteger()) : arg.asFloat();
            greater = argFloat > maxFloat;
        }
        
        if (greater) {
            maxVal = arg;
        }
    }
    
    return maxVal;
}

Value nativeSum(VM& vm, const std::vector<Value>& args) {
    if (args.empty()) {
        vm.throwException(ExceptionBuilder::typeError("at least 1", "0"));
        return Value();
    }
    
    // Expect sum(iterable[, start])
    if (args.size() > 2) {
        vm.throwException(ExceptionBuilder::typeError("1 or 2", std::to_string(args.size())));
        return Value();
    }
    
    // Collect items from iterable
    std::vector<Value> items;
    if (args[0].isList()) {
        const ListValue* list = vm.getListStorage().getList(args[0].asIndex());
        if (!list) {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid list reference"));
            return Value();
        }
        items = list->getItems();
    } else if (args[0].isTuple()) {
        const TupleValue* tup = vm.getTupleStorage().getTuple(args[0].asIndex());
        if (!tup) {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid tuple reference"));
            return Value();
        }
        items = tup->getItems();
    } else if (args[0].isSet()) {
        const SetValue* setv = vm.getSetStorage().getSet(args[0].asIndex());
        if (!setv) {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid set reference"));
            return Value();
        }
        items = setv->getItems();
    } else if (args[0].isString()) {
        // Strings are not treated as numeric iterables for sum
        vm.throwException(ExceptionBuilder::typeError("iterable of numbers", "string"));
        return Value();
    } else {
        vm.throwException(ExceptionBuilder::typeError("iterable", "non-iterable"));
        return Value();
    }
    
    bool anyFloat = false;
    int64_t sumInt = 0;
    double sumFloat = 0.0;
    
    // Optional start value
    if (args.size() == 2) {
        const Value& start = args[1];
        if (start.isInteger()) {
            sumInt += start.asInteger();
        } else if (start.isFloat()) {
            anyFloat = true;
            sumFloat += start.asFloat();
        } else {
            vm.throwException(ExceptionBuilder::typeError("number", "non-number"));
            return Value();
        }
    }
    
    for (const auto& item : items) {
        if (item.isInteger()) {
            sumInt += item.asInteger();
        } else if (item.isFloat()) {
            anyFloat = true;
            sumFloat += item.asFloat();
        } else {
            vm.throwException(ExceptionBuilder::typeError("number", "non-number"));
            return Value();
        }
    }
    if (anyFloat) {
        sumFloat += static_cast<double>(sumInt);
        return Value(sumFloat);
    } else {
        return Value(sumInt);
    }
}

// String functions
Value nativeStr(VM& /*vm*/, const std::vector<Value>& args) {
    if (args.size() != 1) {
        return Value(); // Return nil for invalid arguments
    }
    
    return Value(valueToString(args[0]));
}

// Conversion functions
Value nativeInt(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 1) {
        vm.throwException(ExceptionBuilder::typeError("1", std::to_string(args.size())));
        return Value();
    }

    const Value& arg = args[0];
    if (arg.isInteger()) {
        return arg; // Already integer
    } else if (arg.isFloat()) {
        // Truncate toward zero like CPython int()
        return Value(static_cast<int64_t>(arg.asFloat()));
    } else if (arg.isString()) {
        const std::string original = arg.asString();
        std::string s = StringUtils::trim(original);

        if (s.empty()) {
            vm.throwException(ExceptionBuilder::valueError("invalid literal for int() with base 10: '" + original + "'"));
            return Value();
        }

        int sign = 1;
        size_t pos = 0;
        if (s[0] == '+') {
            pos = 1;
        } else if (s[0] == '-') {
            sign = -1;
            pos = 1;
        }

        if (pos >= s.size()) {
            vm.throwException(ExceptionBuilder::valueError("invalid literal for int() with base 10: '" + original + "'"));
            return Value();
        }

        int64_t result = 0;
        for (; pos < s.size(); ++pos) {
            char c = s[pos];
            if (!StringUtils::isDigit(c)) {
                vm.throwException(ExceptionBuilder::valueError("invalid literal for int() with base 10: '" + original + "'"));
                return Value();
            }
            result = result * 10 + static_cast<int64_t>(c - '0');
        }

        result *= sign;
        return Value(result);
    } else {
        vm.throwException(ExceptionBuilder::typeError("number or string", "other"));
        return Value();
    }
}

Value nativeSubstr(VM& vm, const std::vector<Value>& args) {
    if (args.size() < 2 || args.size() > 3) {
        vm.throwException(ExceptionBuilder::typeError("2 or 3", std::to_string(args.size())));
        return Value();
    }
    
    if (!args[0].isString()) {
        vm.throwException(ExceptionBuilder::typeError("string", "non-string"));
        return Value();
    }
    
    if (!args[1].isInteger()) {
        vm.throwException(ExceptionBuilder::typeError("integer", "non-integer"));
        return Value();
    }
    
    const std::string& str = args[0].asString();
    int64_t start = args[1].asInteger();
    
    if (start < 0) {
        start = 0;
    }
    
    if (start >= static_cast<int64_t>(str.length())) {
        return Value(""); // Return empty string if start is beyond the string
    }
    
    size_t length = str.length() - start;
    if (args.size() == 3) {
        if (!args[2].isInteger()) {
            vm.throwException(ExceptionBuilder::typeError("integer", "non-integer"));
            return Value();
        }
        
        int64_t len = args[2].asInteger();
        if (len < 0) {
            length = 0;
        } else if (static_cast<size_t>(len) < length) {
            length = static_cast<size_t>(len);
        }
    }
    
    return Value(str.substr(static_cast<size_t>(start), length));
}

Value nativeAny(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 1) {
        vm.throwException(ExceptionBuilder::typeError("1", std::to_string(args.size())));
        return Value();
    }
    const Value& arg = args[0];
    if (arg.isList()) {
        const ListValue* list = vm.getListStorage().getList(arg.asIndex());
        if (!list) { vm.throwException(ExceptionBuilder::runtimeError("Invalid list reference")); return Value(); }
        for (const auto& item : list->getItems()) {
            if (isTruthy(vm, item)) return Value(true);
        }
        return Value(false);
    } else if (arg.isTuple()) {
        const TupleValue* tup = vm.getTupleStorage().getTuple(arg.asIndex());
        if (!tup) { vm.throwException(ExceptionBuilder::runtimeError("Invalid tuple reference")); return Value(); }
        for (const auto& item : tup->getItems()) {
            if (isTruthy(vm, item)) return Value(true);
        }
        return Value(false);
    } else if (arg.isSet()) {
        const SetValue* setv = vm.getSetStorage().getSet(arg.asIndex());
        if (!setv) { vm.throwException(ExceptionBuilder::runtimeError("Invalid set reference")); return Value(); }
        for (const auto& item : setv->getItems()) {
            if (isTruthy(vm, item)) return Value(true);
        }
        return Value(false);
    } else if (arg.isDict()) {
        const DictValue* dict = vm.getDictStorage().getDict(arg.asIndex());
        if (!dict) { vm.throwException(ExceptionBuilder::runtimeError("Invalid dictionary reference")); return Value(); }
        for (const auto& key : dict->getKeys()) {
            if (isTruthy(vm, Value(key))) return Value(true);
        }
        return Value(false);
    } else if (arg.isString()) {
        const std::string s = arg.asString();
        if (s.empty()) return Value(false);
        for (char c : s) {
            if (isTruthy(vm, Value(std::string(1, c)))) return Value(true);
        }
        return Value(false);
    }
    vm.throwException(ExceptionBuilder::typeError("iterable", "other"));
    return Value();
}

Value nativeAll(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 1) {
        vm.throwException(ExceptionBuilder::typeError("1", std::to_string(args.size())));
        return Value();
    }
    const Value& arg = args[0];
    if (arg.isList()) {
        const ListValue* list = vm.getListStorage().getList(arg.asIndex());
        if (!list) { vm.throwException(ExceptionBuilder::runtimeError("Invalid list reference")); return Value(); }
        for (const auto& item : list->getItems()) {
            if (!isTruthy(vm, item)) return Value(false);
        }
        return Value(true);
    } else if (arg.isTuple()) {
        const TupleValue* tup = vm.getTupleStorage().getTuple(arg.asIndex());
        if (!tup) { vm.throwException(ExceptionBuilder::runtimeError("Invalid tuple reference")); return Value(); }
        for (const auto& item : tup->getItems()) {
            if (!isTruthy(vm, item)) return Value(false);
        }
        return Value(true);
    } else if (arg.isSet()) {
        const SetValue* setv = vm.getSetStorage().getSet(arg.asIndex());
        if (!setv) { vm.throwException(ExceptionBuilder::runtimeError("Invalid set reference")); return Value(); }
        for (const auto& item : setv->getItems()) {
            if (!isTruthy(vm, item)) return Value(false);
        }
        return Value(true);
    } else if (arg.isDict()) {
        const DictValue* dict = vm.getDictStorage().getDict(arg.asIndex());
        if (!dict) { vm.throwException(ExceptionBuilder::runtimeError("Invalid dictionary reference")); return Value(); }
        for (const auto& key : dict->getKeys()) {
            if (!isTruthy(vm, Value(key))) return Value(false);
        }
        return Value(true);
    } else if (arg.isString()) {
        const std::string s = arg.asString();
        for (char c : s) {
            if (!isTruthy(vm, Value(std::string(1, c)))) return Value(false);
        }
        return Value(!s.empty());
    }
    vm.throwException(ExceptionBuilder::typeError("iterable", "other"));
    return Value();
}

// List functions
Value nativeAppend(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 2) {
        vm.throwException(ExceptionBuilder::typeError("2", std::to_string(args.size())));
        return Value();
    }
    
    if (!args[0].isList()) {
        vm.throwException(ExceptionBuilder::typeError("list", "non-list"));
        return Value();
    }
    
    uint32_t listIndex = args[0].asIndex();
    ListValue* list = vm.getListStorage().getList(listIndex);
    
    if (!list) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid list reference"));
        return Value();
    }
    
    list->append(args[1]);
    return args[0]; // Return the list
}

Value nativeRemove(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 2) {
        vm.throwException(ExceptionBuilder::typeError("2", std::to_string(args.size())));
        return Value();
    }
    
    if (!args[0].isList()) {
        vm.throwException(ExceptionBuilder::typeError("list", "non-list"));
        return Value();
    }
    
    uint32_t listIndex = args[0].asIndex();
    ListValue* list = vm.getListStorage().getList(listIndex);
    
    if (!list) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid list reference"));
        return Value();
    }
    
    const Value& target = args[1];
    const auto& items = list->getItems();
    size_t pos = items.size();
    for (size_t i = 0; i < items.size(); ++i) {
        if (Value::valuesEqual(items[i], target)) {
            pos = i;
            break;
        }
    }
    if (pos == items.size()) {
        vm.throwException(ExceptionBuilder::valueError("list.remove(x): x not in list"));
        return Value();
    }
    if (!list->remove(pos)) {
        vm.throwException(ExceptionBuilder::runtimeError("Failed to remove element"));
        return Value();
    }
    return args[0]; // Return the list
}

Value nativeExtend(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 2) {
        vm.throwException(ExceptionBuilder::typeError("2", std::to_string(args.size())));
        return Value();
    }
    
    if (!args[0].isList()) {
        vm.throwException(ExceptionBuilder::typeError("list", "non-list"));
        return Value();
    }
    
    uint32_t listIndex1 = args[0].asIndex();
    ListValue* list1 = vm.getListStorage().getList(listIndex1);
    if (!list1) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid list reference"));
        return Value();
    }
    
    // Extend with any iterable: list/tuple/set/string/dict(keys)
    if (args[1].isList()) {
        const ListValue* list2 = vm.getListStorage().getList(args[1].asIndex());
        if (!list2) {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid list reference"));
            return Value();
        }
        for (const auto& item : list2->getItems()) {
            list1->append(item);
        }
    } else if (args[1].isTuple()) {
        const TupleValue* tup = vm.getTupleStorage().getTuple(args[1].asIndex());
        if (!tup) {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid tuple reference"));
            return Value();
        }
        for (const auto& item : tup->getItems()) {
            list1->append(item);
        }
    } else if (args[1].isSet()) {
        const SetValue* setv = vm.getSetStorage().getSet(args[1].asIndex());
        if (!setv) {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid set reference"));
            return Value();
        }
        for (const auto& item : setv->getItems()) {
            list1->append(item);
        }
    } else if (args[1].isString()) {
        const std::string s = args[1].asString();
        for (char c : s) {
            list1->append(Value(std::string(1, c)));
        }
    } else if (args[1].isDict()) {
        const DictValue* dict = vm.getDictStorage().getDict(args[1].asIndex());
        if (!dict) {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid dictionary reference"));
            return Value();
        }
        // Extend with keys by default
        for (const auto& kv : dict->getItems()) {
            list1->append(Value(kv.first));
        }
    } else {
        vm.throwException(ExceptionBuilder::typeError("iterable", "non-iterable"));
        return Value();
    }
    
    return args[0]; // Return the first list
}

Value nativeInsert(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 3) {
        vm.throwException(ExceptionBuilder::typeError("3", std::to_string(args.size())));
        return Value();
    }
    
    if (!args[0].isList()) {
        vm.throwException(ExceptionBuilder::typeError("list", "non-list"));
        return Value();
    }
    
    if (!args[1].isInteger()) {
        vm.throwException(ExceptionBuilder::typeError("integer", "non-integer"));
        return Value();
    }
    
    uint32_t listIndex = args[0].asIndex();
    ListValue* list = vm.getListStorage().getList(listIndex);
    
    if (!list) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid list reference"));
        return Value();
    }
    
    int64_t index = args[1].asInteger();
    // Normalize negative index relative to list size, then clamp
    if (index < 0) {
        index += static_cast<int64_t>(list->size());
    }
    if (index < 0) {
        index = 0;
    }
    if (index > static_cast<int64_t>(list->size())) {
        index = static_cast<int64_t>(list->size());
    }
    
    if (!list->insert(static_cast<size_t>(index), args[2])) {
        vm.throwException(ExceptionBuilder::runtimeError("Failed to insert element"));
        return Value();
    }
    
    return args[0]; // Return the list
}

Value nativePop(VM& vm, const std::vector<Value>& args) {
    if (args.size() < 1 || args.size() > 2) {
        vm.throwException(ExceptionBuilder::typeError("1 or 2", std::to_string(args.size())));
        return Value();
    }
    
    if (!args[0].isList()) {
        vm.throwException(ExceptionBuilder::typeError("list", "non-list"));
        return Value();
    }
    
    uint32_t listIndex = args[0].asIndex();
    ListValue* list = vm.getListStorage().getList(listIndex);
    
    if (!list) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid list reference"));
        return Value();
    }
    
    if (list->empty()) {
        vm.throwException(ExceptionBuilder::indexError(static_cast<size_t>(0), list->size()));
        return Value();
    }
    
    size_t index;
    if (args.size() == 1) {
        // Default: pop the last element
        index = list->size() - 1;
    } else {
        // Pop at specified index
        if (!args[1].isInteger()) {
            vm.throwException(ExceptionBuilder::typeError("integer", "non-integer"));
            return Value();
        }
        
        int64_t idx = args[1].asInteger();
        if (idx < 0) {
            idx += static_cast<int64_t>(list->size());
        }
        if (idx < 0 || idx >= static_cast<int64_t>(list->size())) {
            vm.throwException(ExceptionBuilder::indexError(static_cast<size_t>(idx < 0 ? 0 : idx), list->size()));
            return Value();
        }
        index = static_cast<size_t>(idx);
    }
    
    Value result = list->get(index);
    if (!list->remove(index)) {
        vm.throwException(ExceptionBuilder::runtimeError("Failed to remove element"));
        return Value();
    }
    
    return result; // Return the popped element
}

Value nativeClear(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 1) {
        vm.throwException(ExceptionBuilder::typeError("1", std::to_string(args.size())));
        return Value();
    }
    
    if (args[0].isList()) {
        uint32_t listIndex = args[0].asIndex();
        ListValue* list = vm.getListStorage().getList(listIndex);
        
        if (!list) {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid list reference"));
            return Value();
        }
        
        // Create a new empty list to replace the old one
        *list = ListValue();
        return args[0]; // Return the list
    } else if (args[0].isDict()) {
        uint32_t dictIndex = args[0].asIndex();
        DictValue* dict = vm.getDictStorage().getDict(dictIndex);
        
        if (!dict) {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid dictionary reference"));
            return Value();
        }
        
        dict->clear();
        return args[0]; // Return the dictionary
    } else {
        vm.throwException(ExceptionBuilder::typeError("list or dict", "other"));
        return Value();
    }
}

Value nativeSort(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 1) {
        vm.throwException(ExceptionBuilder::typeError("1", std::to_string(args.size())));
        return Value();
    }
    
    if (!args[0].isList()) {
        vm.throwException(ExceptionBuilder::typeError("list", "non-list"));
        return Value();
    }
    
    uint32_t listIndex = args[0].asIndex();
    ListValue* list = vm.getListStorage().getList(listIndex);
    
    if (!list) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid list reference"));
        return Value();
    }
    
    // Get all items
    std::vector<Value> items = list->getItems();
    
    // Sort the items
    std::sort(items.begin(), items.end(), [](const Value& a, const Value& b) {
        // Simple comparison: numbers < strings < booleans < nil < functions < lists < dicts
        if (a.isInteger() && b.isInteger()) {
            return a.asInteger() < b.asInteger();
        } else if (a.isFloat() && b.isFloat()) {
            return a.asFloat() < b.asFloat();
        } else if ((a.isInteger() || a.isFloat()) && (b.isInteger() || b.isFloat())) {
            double aVal = a.isInteger() ? static_cast<double>(a.asInteger()) : a.asFloat();
            double bVal = b.isInteger() ? static_cast<double>(b.asInteger()) : b.asFloat();
            return aVal < bVal;
        } else if (a.isString() && b.isString()) {
            return a.asString() < b.asString();
        } else {
            // Different types: use a predefined order
            int aTypeOrder = getTypeOrder(a);
            int bTypeOrder = getTypeOrder(b);
            return aTypeOrder < bTypeOrder;
        }
    });
    
    // Create a new list with the sorted items
    *list = ListValue(items);
    
    return args[0]; // Return the list
}

Value nativeReverse(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 1) {
        vm.throwException(ExceptionBuilder::typeError("1", std::to_string(args.size())));
        return Value();
    }
    
    if (!args[0].isList()) {
        vm.throwException(ExceptionBuilder::typeError("list", "non-list"));
        return Value();
    }
    
    uint32_t listIndex = args[0].asIndex();
    ListValue* list = vm.getListStorage().getList(listIndex);
    
    if (!list) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid list reference"));
        return Value();
    }
    
    // Get all items and reverse them
    std::vector<Value> items = list->getItems();
    std::reverse(items.begin(), items.end());
    
    // Create a new list with the reversed items
    *list = ListValue(items);
    
    return args[0]; // Return the list
}

Value nativeSorted(VM& vm, const std::vector<Value>& args) {
    if (args.size() < 1 || args.size() > 2) {
        vm.throwException(ExceptionBuilder::typeError("1 or 2", std::to_string(args.size())));
        return Value();
    }
    
    // Gather items from iterable
    std::vector<Value> items;
    if (args[0].isList()) {
        const ListValue* list = vm.getListStorage().getList(args[0].asIndex());
        if (!list) {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid list reference"));
            return Value();
        }
        items = list->getItems();
    } else if (args[0].isTuple()) {
        const TupleValue* tup = vm.getTupleStorage().getTuple(args[0].asIndex());
        if (!tup) {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid tuple reference"));
            return Value();
        }
        items = tup->getItems();
    } else if (args[0].isSet()) {
        const SetValue* setv = vm.getSetStorage().getSet(args[0].asIndex());
        if (!setv) {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid set reference"));
            return Value();
        }
        items = setv->getItems();
    } else if (args[0].isString()) {
        const std::string s = args[0].asString();
        for (char c : s) items.push_back(Value(std::string(1, c)));
    } else {
        vm.throwException(ExceptionBuilder::typeError("iterable", "non-iterable"));
        return Value();
    }
    
    std::sort(items.begin(), items.end(), [](const Value& a, const Value& b) {
        if (a.isInteger() && b.isInteger()) {
            return a.asInteger() < b.asInteger();
        } else if (a.isFloat() && b.isFloat()) {
            return a.asFloat() < b.asFloat();
        } else if ((a.isInteger() || a.isFloat()) && (b.isInteger() || b.isFloat())) {
            double aVal = a.isInteger() ? static_cast<double>(a.asInteger()) : a.asFloat();
            double bVal = b.isInteger() ? static_cast<double>(b.asInteger()) : b.asFloat();
            return aVal < bVal;
        } else if (a.isString() && b.isString()) {
            return a.asString() < b.asString();
        } else {
            int aTypeOrder = getTypeOrder(a);
            int bTypeOrder = getTypeOrder(b);
            return aTypeOrder < bTypeOrder;
        }
    });
    // Optional reverse flag
    bool reverse = false;
    if (args.size() == 2) {
        if (!args[1].isBoolean()) {
            vm.throwException(ExceptionBuilder::typeError("boolean", "non-boolean"));
            return Value();
        }
        reverse = args[1].asBoolean();
    }
    if (reverse) {
        std::reverse(items.begin(), items.end());
    }
    size_t newListIndex = vm.getListStorage().createList(items);
    return Value(static_cast<uint32_t>(newListIndex), ValueType::LIST);
}

Value nativeReversed(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 1) {
        vm.throwException(ExceptionBuilder::typeError("1", std::to_string(args.size())));
        return Value();
    }
    std::vector<Value> items;
    if (args[0].isList()) {
        const ListValue* list = vm.getListStorage().getList(args[0].asIndex());
        if (!list) {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid list reference"));
            return Value();
        }
        items = list->getItems();
    } else if (args[0].isTuple()) {
        const TupleValue* tup = vm.getTupleStorage().getTuple(args[0].asIndex());
        if (!tup) {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid tuple reference"));
            return Value();
        }
        items = tup->getItems();
    } else if (args[0].isSet()) {
        const SetValue* setv = vm.getSetStorage().getSet(args[0].asIndex());
        if (!setv) {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid set reference"));
            return Value();
        }
        items = setv->getItems();
    } else if (args[0].isString()) {
        const std::string s = args[0].asString();
        for (char c : s) items.push_back(Value(std::string(1, c)));
    } else {
        vm.throwException(ExceptionBuilder::typeError("iterable", "non-iterable"));
        return Value();
    }
    std::reverse(items.begin(), items.end());
    size_t newListIndex = vm.getListStorage().createList(items);
    return Value(static_cast<uint32_t>(newListIndex), ValueType::LIST);
}

Value nativeCount(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 2) {
        vm.throwException(ExceptionBuilder::typeError("2", std::to_string(args.size())));
        return Value();
    }
    
    if (!args[0].isList()) {
        vm.throwException(ExceptionBuilder::typeError("list", "non-list"));
        return Value();
    }
    
    uint32_t listIndex = args[0].asIndex();
    ListValue* list = vm.getListStorage().getList(listIndex);
    
    if (!list) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid list reference"));
        return Value();
    }
    
    Value target = args[1];
    const std::vector<Value>& items = list->getItems();
    
    int64_t count = 0;
    for (const auto& item : items) {
        if (Value::valuesEqual(item, target)) {
            count++;
        }
    }
    
    return Value(count);
}

Value nativeIndex(VM& vm, const std::vector<Value>& args) {
    if (args.size() < 2 || args.size() > 3) {
        vm.throwException(ExceptionBuilder::typeError("2 or 3", std::to_string(args.size())));
        return Value();
    }
    
    if (!args[0].isList()) {
        vm.throwException(ExceptionBuilder::typeError("list", "non-list"));
        return Value();
    }
    
    uint32_t listIndex = args[0].asIndex();
    ListValue* list = vm.getListStorage().getList(listIndex);
    
    if (!list) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid list reference"));
        return Value();
    }
    
    Value target = args[1];
    const std::vector<Value>& items = list->getItems();
    
    size_t start = 0;
    if (args.size() == 3) {
        if (!args[2].isInteger()) {
            vm.throwException(ExceptionBuilder::typeError("integer", "non-integer"));
            return Value();
        }
        
        int64_t startIdx = args[2].asInteger();
        if (startIdx < 0) {
            startIdx += static_cast<int64_t>(items.size());
        }
        
        if (startIdx < 0) {
            start = 0;
        } else if (static_cast<size_t>(startIdx) >= items.size()) {
            vm.throwException(ExceptionBuilder::valueError("Value not found in list"));
            return Value();
        } else {
            start = static_cast<size_t>(startIdx);
        }
    }
    
    for (size_t i = start; i < items.size(); ++i) {
        if (Value::valuesEqual(items[i], target)) {
            return Value(static_cast<int64_t>(i));
        }
    }
    
    vm.throwException(ExceptionBuilder::valueError("Value not found in list"));
    return Value();
}

Value nativeListCopy(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 1) {
        vm.throwException(ExceptionBuilder::typeError("1", std::to_string(args.size())));
        return Value();
    }
    
    if (!args[0].isList()) {
        vm.throwException(ExceptionBuilder::typeError("list", "non-list"));
        return Value();
    }
    
    uint32_t listIndex = args[0].asIndex();
    ListValue* list = vm.getListStorage().getList(listIndex);
    
    if (!list) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid list reference"));
        return Value();
    }
    
    // Create a new list with the same items
    const std::vector<Value>& items = list->getItems();
    size_t newListIndex = vm.getListStorage().createList(items);
    
    return Value(static_cast<uint32_t>(newListIndex), ValueType::LIST);
}

// Dictionary functions
Value nativeKeys(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 1) {
        vm.throwException(ExceptionBuilder::typeError("1", std::to_string(args.size())));
        return Value();
    }
    
    if (!args[0].isDict()) {
        vm.throwException(ExceptionBuilder::typeError("dict", "non-dict"));
        return Value();
    }
    
    uint32_t dictIndex = args[0].asIndex();
    const DictValue* dict = vm.getDictStorage().getDict(dictIndex);
    
    if (!dict) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid dictionary reference"));
        return Value();
    }
    
    std::vector<std::string> keys = dict->getKeys();
    
    // Create a new list with the keys
    size_t listIndex = vm.getListStorage().createList(keys.size());
    ListValue* list = vm.getListStorage().getList(listIndex);
    
    for (const auto& key : keys) {
        list->append(Value(key));
    }
    
    return Value(static_cast<uint32_t>(listIndex), ValueType::LIST);
}

Value nativeValues(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 1) {
        vm.throwException(ExceptionBuilder::typeError("1", std::to_string(args.size())));
        return Value();
    }
    
    if (!args[0].isDict()) {
        vm.throwException(ExceptionBuilder::typeError("dict", "non-dict"));
        return Value();
    }
    
    uint32_t dictIndex = args[0].asIndex();
    const DictValue* dict = vm.getDictStorage().getDict(dictIndex);
    
    if (!dict) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid dictionary reference"));
        return Value();
    }
    
    std::vector<Value> values = dict->getValues();
    
    // Create a new list with the values
    size_t listIndex = vm.getListStorage().createList(values);
    
    return Value(static_cast<uint32_t>(listIndex), ValueType::LIST);
}

Value nativeContains(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 2) {
        vm.throwException(ExceptionBuilder::typeError("2", std::to_string(args.size())));
        return Value();
    }
    
    if (!args[0].isDict()) {
        vm.throwException(ExceptionBuilder::typeError("dict", "non-dict"));
        return Value();
    }
    
    if (!args[1].isString()) {
        vm.throwException(ExceptionBuilder::typeError("string", "non-string"));
        return Value();
    }
    
    uint32_t dictIndex = args[0].asIndex();
    const DictValue* dict = vm.getDictStorage().getDict(dictIndex);
    
    if (!dict) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid dictionary reference"));
        return Value();
    }
    
    return Value(dict->contains(args[1].asString()));
}

Value nativeUpdate(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 2) {
        vm.throwException(ExceptionBuilder::typeError("2", std::to_string(args.size())));
        return Value();
    }
    
    if (!args[0].isDict() || !args[1].isDict()) {
        vm.throwException(ExceptionBuilder::typeError("dict", "non-dict"));
        return Value();
    }
    
    uint32_t dictIndex1 = args[0].asIndex();
    uint32_t dictIndex2 = args[1].asIndex();
    
    DictValue* dict1 = vm.getDictStorage().getDict(dictIndex1);
    const DictValue* dict2 = vm.getDictStorage().getDict(dictIndex2);
    
    if (!dict1 || !dict2) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid dictionary reference"));
        return Value();
    }
    
    // Get all key-value pairs from dict2 and add them to dict1
    std::vector<std::pair<std::string, Value>> items = dict2->getItems();
    for (const auto& item : items) {
        dict1->set(item.first, item.second);
    }
    
    return args[0]; // Return the first dictionary
}

Value nativeGet(VM& vm, const std::vector<Value>& args) {
    if (args.size() < 2 || args.size() > 3) {
        vm.throwException(ExceptionBuilder::typeError("2 or 3", std::to_string(args.size())));
        return Value();
    }
    
    if (!args[0].isDict()) {
        vm.throwException(ExceptionBuilder::typeError("dict", "non-dict"));
        return Value();
    }
    
    if (!args[1].isString()) {
        vm.throwException(ExceptionBuilder::typeError("string", "non-string"));
        return Value();
    }
    
    uint32_t dictIndex = args[0].asIndex();
    const DictValue* dict = vm.getDictStorage().getDict(dictIndex);
    
    if (!dict) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid dictionary reference"));
        return Value();
    }
    
    std::string key = args[1].asString();
    
    if (args.size() == 2) {
        // No default value provided
        return dict->get(key);
    } else {
        // Default value provided
        return dict->get(key, args[2]);
    }
}

Value nativeCopy(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 1) {
        vm.throwException(ExceptionBuilder::typeError("1", std::to_string(args.size())));
        return Value();
    }
    
    if (!args[0].isDict()) {
        vm.throwException(ExceptionBuilder::typeError("dict", "non-dict"));
        return Value();
    }
    
    uint32_t dictIndex = args[0].asIndex();
    const DictValue* dict = vm.getDictStorage().getDict(dictIndex);
    
    if (!dict) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid dictionary reference"));
        return Value();
    }
    
    // Get all key-value pairs from the original dictionary
    std::vector<std::pair<std::string, Value>> items = dict->getItems();
    
    // Create a new dictionary with the same items
    size_t newDictIndex = vm.getDictStorage().createDict(items);
    
    return Value(static_cast<uint32_t>(newDictIndex), ValueType::DICT);
}

Value nativeFromKeys(VM& vm, const std::vector<Value>& args) {
    if (args.size() < 1 || args.size() > 2) {
        vm.throwException(ExceptionBuilder::typeError("1 or 2", std::to_string(args.size())));
        return Value();
    }
    
    // Default value is nil if not provided
    Value defaultValue = args.size() > 1 ? args[1] : Value();
    
    // Check if the first argument is a list
    if (!args[0].isList()) {
        vm.throwException(ExceptionBuilder::typeError("list", "non-list"));
        return Value();
    }
    
    uint32_t listIndex = args[0].asIndex();
    const ListValue* list = vm.getListStorage().getList(listIndex);
    
    if (!list) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid list reference"));
        return Value();
    }
    
    // Create a new dictionary with keys from the list and the default value
    std::vector<std::pair<std::string, Value>> items;
    const std::vector<Value>& listItems = list->getItems();
    
    for (const auto& item : listItems) {
        if (!item.isString()) {
            vm.throwException(ExceptionBuilder::typeError("string", "non-string key"));
            return Value();
        }
        
        items.push_back(std::make_pair(item.asString(), defaultValue));
    }
    
    size_t newDictIndex = vm.getDictStorage().createDict(items);
    return Value(static_cast<uint32_t>(newDictIndex), ValueType::DICT);
}

Value nativeItems(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 1) {
        vm.throwException(ExceptionBuilder::typeError("1", std::to_string(args.size())));
        return Value();
    }
    
    if (!args[0].isDict()) {
        vm.throwException(ExceptionBuilder::typeError("dict", "non-dict"));
        return Value();
    }
    
    uint32_t dictIndex = args[0].asIndex();
    const DictValue* dict = vm.getDictStorage().getDict(dictIndex);
    
    if (!dict) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid dictionary reference"));
        return Value();
    }
    
    // Get all key-value pairs from the dictionary
    std::vector<std::pair<std::string, Value>> dictItems = dict->getItems();
    
    // Create a list of lists, where each inner list is [key, value]
    std::vector<Value> resultItems;
    
    for (const auto& item : dictItems) {
        // Create a list for the key-value pair
        std::vector<Value> pair;
        pair.push_back(Value(item.first));  // Key as string
        pair.push_back(item.second);        // Value
        
        // Create a list object for the pair
        size_t pairIndex = vm.getListStorage().createList(pair);
        resultItems.push_back(Value(static_cast<uint32_t>(pairIndex), ValueType::LIST));
    }
    
    // Create the result list containing all pairs
    size_t resultIndex = vm.getListStorage().createList(resultItems);
    return Value(static_cast<uint32_t>(resultIndex), ValueType::LIST);
}

Value nativeDictPop(VM& vm, const std::vector<Value>& args) {
    if (args.size() < 1 || args.size() > 2) {
        vm.throwException(ExceptionBuilder::typeError("1 or 2", std::to_string(args.size())));
        return Value();
    }
    
    if (!args[0].isDict()) {
        vm.throwException(ExceptionBuilder::typeError("dict", "non-dict"));
        return Value();
    }
    
    if (args.size() > 1 && !args[1].isString()) {
        vm.throwException(ExceptionBuilder::typeError("string", "non-string"));
        return Value();
    }
    
    uint32_t dictIndex = args[0].asIndex();
    DictValue* dict = vm.getDictStorage().getDict(dictIndex);
    
    if (!dict) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid dictionary reference"));
        return Value();
    }
    
    // If no key is provided, remove and return an arbitrary element
    if (args.size() == 1) {
        if (dict->empty()) {
            vm.throwException(ExceptionBuilder::runtimeError("pop from an empty dictionary"));
            return Value();
        }
        
        // Get the first key-value pair
        std::vector<std::pair<std::string, Value>> items = dict->getItems();
        if (items.empty()) {
            vm.throwException(ExceptionBuilder::runtimeError("pop from an empty dictionary"));
            return Value();
        }
        
        // Remove the first item and return its value
        std::string key = items[0].first;
        Value value = dict->get(key);
        dict->remove(key);
        return value;
    } else {
        // Remove and return the value for the specified key
        std::string key = args[1].asString();
        
        if (!dict->contains(key)) {
            vm.throwException(ExceptionBuilder::runtimeError("Key '" + key + "' not found"));
            return Value();
        }
        
        Value value = dict->get(key);
        dict->remove(key);
        return value;
    }
}

Value nativePopItem(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 1) {
        vm.throwException(ExceptionBuilder::typeError("1", std::to_string(args.size())));
        return Value();
    }
    
    if (!args[0].isDict()) {
        vm.throwException(ExceptionBuilder::typeError("dict", "non-dict"));
        return Value();
    }
    
    uint32_t dictIndex = args[0].asIndex();
    DictValue* dict = vm.getDictStorage().getDict(dictIndex);
    
    if (!dict) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid dictionary reference"));
        return Value();
    }
    
    if (dict->empty()) {
        vm.throwException(ExceptionBuilder::runtimeError("popitem from an empty dictionary"));
        return Value();
    }
    
    // Get the first key-value pair
    std::vector<std::pair<std::string, Value>> items = dict->getItems();
    if (items.empty()) {
        vm.throwException(ExceptionBuilder::runtimeError("popitem from an empty dictionary"));
        return Value();
    }
    
    // Get the first item
    std::string key = items[0].first;
    Value value = dict->get(key);
    
    // Remove the item from the dictionary
    dict->remove(key);
    
    // Create a list for the key-value pair
    std::vector<Value> pair;
    pair.push_back(Value(key));  // Key as string
    pair.push_back(value);       // Value
    
    // Create a list object for the pair
    size_t pairIndex = vm.getListStorage().createList(pair);
    return Value(static_cast<uint32_t>(pairIndex), ValueType::LIST);
}

Value nativeSetDefault(VM& vm, const std::vector<Value>& args) {
    if (args.size() < 2 || args.size() > 3) {
        vm.throwException(ExceptionBuilder::typeError("2 or 3", std::to_string(args.size())));
        return Value();
    }
    
    if (!args[0].isDict()) {
        vm.throwException(ExceptionBuilder::typeError("dict", "non-dict"));
        return Value();
    }
    
    if (!args[1].isString()) {
        vm.throwException(ExceptionBuilder::typeError("string", "non-string"));
        return Value();
    }
    
    uint32_t dictIndex = args[0].asIndex();
    DictValue* dict = vm.getDictStorage().getDict(dictIndex);
    
    if (!dict) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid dictionary reference"));
        return Value();
    }
    
    std::string key = args[1].asString();
    
    // If key is in dictionary, return its value
    if (dict->contains(key)) {
        return dict->get(key);
    }
    
    // Otherwise, set key to default value and return the default value
    Value defaultValue = args.size() > 2 ? args[2] : Value();
    dict->set(key, defaultValue);
    
    return defaultValue;
}

// __import_all__(module_dict) -> count imported
Value nativeImportAll(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 1) {
        vm.throwException(ExceptionBuilder::typeError("1", std::to_string(args.size())));
        return Value();
    }
    if (!args[0].isDict()) {
        vm.throwException(ExceptionBuilder::typeError("dict", "non-dict"));
        return Value();
    }

    uint32_t dictIndex = args[0].asIndex();
    const DictValue* dict = vm.getDictStorage().getDict(dictIndex);
    if (!dict) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid dictionary reference"));
        return Value();
    }

    // Determine names to import
    std::vector<std::string> names;
    Value allVal = dict->get("__all__");
    if (allVal.isList()) {
        uint32_t listIndex = allVal.asIndex();
        const ListValue* list = vm.getListStorage().getList(listIndex);
        if (!list) {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid list reference in __all__"));
            return Value();
        }
        for (size_t i = 0; i < list->size(); ++i) {
            Value item = list->get(i);
            if (!item.isString()) {
                vm.throwException(ExceptionBuilder::typeError("string", "non-string in __all__"));
                return Value();
            }
            names.push_back(item.asString());
        }
    } else if (allVal.isTuple()) {
        uint32_t tupIndex = allVal.asIndex();
        const TupleValue* tup = vm.getTupleStorage().getTuple(tupIndex);
        if (!tup) {
            vm.throwException(ExceptionBuilder::runtimeError("Invalid tuple reference in __all__"));
            return Value();
        }
        for (size_t i = 0; i < tup->size(); ++i) {
            Value item = tup->get(i);
            if (!item.isString()) {
                vm.throwException(ExceptionBuilder::typeError("string", "non-string in __all__"));
                return Value();
            }
            names.push_back(item.asString());
        }
    } else if (!allVal.isNil()) {
        // __all__ present but not list/tuple
        vm.throwException(ExceptionBuilder::typeError("list/tuple", "invalid __all__ type"));
        return Value();
    } else {
        // Fallback: import all non-private names except metadata
        for (const auto& kv : dict->getItems()) {
            const std::string& key = kv.first;
            if (key == "__name__" || key == "__file__" || key == "__doc__" || key == "__all__") {
                continue;
            }
            if (!key.empty() && key[0] == '_') {
                continue;
            }
            names.push_back(key);
        }
    }

    // Import into caller globals
    for (const auto& n : names) {
        vm.setGlobal(n, dict->get(n));
    }

    return Value(static_cast<int64_t>(names.size()));
}

// __import__(name[, base_path]) -> module dict
Value nativeImport(VM& vm, const std::vector<Value>& args) {
    if (args.empty() || args.size() > 2) {
        vm.throwException(ExceptionBuilder::typeError("1 or 2", std::to_string(args.size())));
        return Value();
    }
    if (!args[0].isString()) {
        vm.throwException(ExceptionBuilder::typeError("string", "non-string"));
        return Value();
    }

    std::string moduleName = args[0].asString();
    std::string basePathArg;
    if (args.size() == 2) {
        if (!args[1].isString()) {
            vm.throwException(ExceptionBuilder::typeError("string", "non-string"));
            return Value();
        }
        basePathArg = args[1].asString();
    }

    // Build candidate relative path: support dotted module names
    std::string relPath = moduleName;
    // Use '/' to replace dots for module path regardless of platform
    std::replace(relPath.begin(), relPath.end(), '.', '/');
    relPath += ".rgb";

    // Build search paths
    std::vector<std::filesystem::path> searchDirs;
    if (!basePathArg.empty()) {
        searchDirs.emplace_back(basePathArg);
    }

    // Env var search path
    const char* envPath = std::getenv("RGLITE_IMPORT_PATH");
    if (envPath && *envPath) {
        std::string sp(envPath);
        char sep = ';';
        // Also accept ':' separators
        size_t pos = 0; 
        while (pos < sp.size()) {
            size_t next = sp.find_first_of(";:", pos);
            std::string token = sp.substr(pos, next == std::string::npos ? std::string::npos : next - pos);
            if (!token.empty()) searchDirs.emplace_back(token);
            if (next == std::string::npos) break; pos = next + 1;
        }
    }

    // Default search dirs
    auto cwd = std::filesystem::current_path();
    searchDirs.push_back(cwd);

    // Also consider parent-of-cwd paths (up to 3 levels) to support out-of-source
    // builds where tests run under build/ or build/tests/unit.
    {
        std::filesystem::path p = cwd;
        for (int i = 0; i < 3; ++i) {
            if (!p.has_parent_path()) break;
            p = p.parent_path();
            searchDirs.push_back(p);
            searchDirs.push_back(p / "tests" / "modules");
            searchDirs.push_back(p / "modules");
            searchDirs.push_back(p / "examples");
        }
    }

    // Find file
    std::filesystem::path found;
    for (const auto& dir : searchDirs) {
        std::filesystem::path candidate = dir / relPath;
        if (std::filesystem::exists(candidate)) { found = candidate; break; }
        // Also try without dotted path expansion
        candidate = dir / (moduleName + std::string(".rgb"));
        if (std::filesystem::exists(candidate)) { found = candidate; break; }
    }

    if (found.empty()) {
        vm.throwException(ExceptionBuilder::runtimeError(std::string("Module not found: ") + moduleName));
        return Value();
    }

    // Read source
    std::ifstream in(found, std::ios::binary);
    if (!in) {
        vm.throwException(ExceptionBuilder::runtimeError(std::string("Failed to open module: ") + found.string()));
        return Value();
    }
    std::stringstream buffer; buffer << in.rdbuf();
    std::string source = buffer.str();

    // Compile pipeline
    auto errorHandler = std::make_shared<StandardErrorHandler>();
    auto lexer = std::make_unique<Lexer>(source, found.string(), errorHandler);
    auto parser = std::make_unique<Parser>(std::move(lexer), errorHandler);
    auto ast = parser->parse();
    if (!ast) {
        vm.throwException(ExceptionBuilder::runtimeError("Failed to parse module"));
        return Value();
    }
    auto semantic = std::make_unique<SemanticAnalyzer>(errorHandler);
    semantic->analyze(ast);
    if (errorHandler->hasErrors()) {
        vm.throwException(ExceptionBuilder::runtimeError("Semantic errors in module"));
        return Value();
    }
    auto cg = createCodeGenerator();
    // Snapshot variable name mapping BEFORE code generation to avoid pollution
    auto savedVarNames = vm.getVariableNameMap();
    cg->setVM(&vm);
    // Convert parser AST (unique_ptr<Stmt>) to shared_ptr<ASTNode> for codegen
    std::shared_ptr<ASTNode> astNode = std::move(ast);
    (void)cg->generate(astNode);
    // Restore caller's variable name mapping immediately after generation
    // because createVariable() during codegen updates VM mapping
    vm.setVariableNameMap(savedVarNames);
    if (cg->hasErrors()) {
        vm.throwException(ExceptionBuilder::runtimeError("Code generation failed for module"));
        return Value();
    }
    auto chunkPtr = cg->getBytecode();
    if (!chunkPtr) {
        vm.throwException(ExceptionBuilder::runtimeError("No bytecode emitted for module"));
        return Value();
    }

    // Snapshot globals before import and set module metadata
    std::unordered_set<std::string> before;
    for (const auto& k : vm.getGlobalKeys()) before.insert(k);
    Value prevName = vm.getGlobal("__name__");
    Value prevFile = vm.getGlobal("__file__");
    Value prevDoc = vm.getGlobal("__doc__");
    vm.setGlobal("__name__", Value(moduleName));
    vm.setGlobal("__file__", Value(found.string()));
    vm.setGlobal("__doc__", Value());

    // Swap code generator and execute module
    CodeGenerator* oldCG = vm.getCodeGenerator();
    vm.setCodeGenerator(cg.get());
    // Apply module's variable name table for the duration of isolated execution
    {
        // Build mapping from the module's variable table
        std::unordered_map<uint32_t, std::string> moduleVarNames;
        for (const auto& p : cg->getVariableNameTable()) {
            moduleVarNames[p.first] = p.second;
        }
        vm.setVariableNameMap(moduleVarNames);
    }
    bool ok = vm.executeChunkIsolated(*chunkPtr, found.string());
    // After isolated execution, restore caller mapping
    vm.setVariableNameMap(savedVarNames);
    // Restore old code generator regardless of success
    vm.setCodeGenerator(oldCG);
    if (!ok) {
        return Value();
    }

    // Build module dict from new globals
    std::vector<std::pair<std::string, Value>> items;
    for (const auto& k : vm.getGlobalKeys()) {
        if (before.find(k) == before.end()) {
            items.emplace_back(k, vm.getGlobal(k));
        }
    }
    // Also include __name__, __file__, __doc__ from module execution
    Value modName = Value(moduleName);
    Value modFile = Value(found.string());
    Value modDoc = vm.getGlobal("__doc__");
    // Overwrite or append
    bool hasName=false, hasFile=false, hasDoc=false;
    for (auto& p : items) {
        if (p.first == "__name__") { p.second = modName; hasName=true; }
        if (p.first == "__file__") { p.second = modFile; hasFile=true; }
        if (p.first == "__doc__") { hasDoc=true; }
    }
    if (!hasName) items.emplace_back("__name__", modName);
    if (!hasFile) items.emplace_back("__file__", modFile);
    if (!hasDoc) items.emplace_back("__doc__", modDoc);

    size_t dictIdx = vm.getDictStorage().createDict(items);
    Value moduleDict(static_cast<uint32_t>(dictIdx), ValueType::DICT);

    // Clean up: remove module symbols from main globals and restore main metadata
    for (const auto& [key, _] : items) {
        vm.eraseGlobal(key);
    }
    vm.setGlobal("__name__", prevName.isString() ? prevName : Value("__main__"));
    vm.setGlobal("__file__", prevFile);
    vm.setGlobal("__doc__", prevDoc);

    // Assign to a global named after the module for convenience
    vm.setGlobal(moduleName, moduleDict);
    return moduleDict;
}

// __import_bind__(module_dict) -> top-level package dict
// Binds a submodule dict into a hierarchical package structure based on its __name__
// Example: __name__ == "sort.merge_sort" will ensure global "sort" is a dict
// and set sort["merge_sort"] = module_dict, returning the "sort" dict.
Value nativeImportBind(VM& vm, const std::vector<Value>& args) {
    if (args.size() != 1 || !args[0].isDict()) {
        vm.throwException(ExceptionBuilder::typeError("dict", args.empty() ? "no args" : "invalid arg for __import_bind__"));
        return Value();
    }

    DictValue* moduleDict = vm.getDictStorage().getDict(args[0].asIndex());
    if (!moduleDict) {
        vm.throwException(ExceptionBuilder::runtimeError("Invalid module dict in __import_bind__"));
        return Value();
    }

    Value nameVal = moduleDict->get("__name__");
    if (!nameVal.isString()) {
        vm.throwException(ExceptionBuilder::typeError("string", "module __name__ missing or non-string"));
        return Value();
    }
    std::string moduleName = nameVal.asString();
    size_t dotPos = moduleName.find('.');
    if (dotPos == std::string::npos) {
        // Not a dotted name; nothing to bind, return module dict as-is
        return args[0];
    }

    // Split name into segments
    std::vector<std::string> parts;
    size_t start = 0;
    while (true) {
        size_t pos = moduleName.find('.', start);
        if (pos == std::string::npos) { parts.push_back(moduleName.substr(start)); break; }
        parts.push_back(moduleName.substr(start, pos - start));
        start = pos + 1;
    }
    if (parts.empty()) {
        vm.throwException(ExceptionBuilder::runtimeError("Failed to parse module name in __import_bind__"));
        return Value();
    }

    const std::string& topName = parts[0];
    // Ensure top-level package dict exists in globals
    Value topVal = vm.getGlobal(topName);
    DictValue* topDict = nullptr;
    if (topVal.isDict()) {
        topDict = vm.getDictStorage().getDict(topVal.asIndex());
    } else {
        size_t topIdx = vm.getDictStorage().createDict(0);
        topDict = vm.getDictStorage().getDict(topIdx);
        vm.setGlobal(topName, Value(static_cast<uint32_t>(topIdx), ValueType::DICT));
    }
    if (!topDict) {
        vm.throwException(ExceptionBuilder::runtimeError("Failed to create or access top-level package dict"));
        return Value();
    }

    // Walk/create intermediate package dicts
    DictValue* current = topDict;
    for (size_t i = 1; i + 1 < parts.size(); ++i) {
        const std::string& seg = parts[i];
        Value segVal = current->get(seg);
        if (segVal.isDict()) {
            current = vm.getDictStorage().getDict(segVal.asIndex());
            if (!current) {
                vm.throwException(ExceptionBuilder::runtimeError("Invalid dict for package segment: " + seg));
                return Value();
            }
        } else {
            size_t idx = vm.getDictStorage().createDict(0);
            DictValue* next = vm.getDictStorage().getDict(idx);
            if (!next) {
                vm.throwException(ExceptionBuilder::runtimeError("Failed to allocate dict for package segment: " + seg));
                return Value();
            }
            current->set(seg, Value(static_cast<uint32_t>(idx), ValueType::DICT));
            current = next;
        }
    }

    // Bind leaf
    const std::string& leaf = parts.back();
    current->set(leaf, args[0]);

    // Return top-level package dict
    return vm.getGlobal(topName);
}

// Register all builtin functions with the VM
void registerBuiltinFunctions(VM& vm) {
    // I/O functions
    vm.registerNativeFunction("print", nativePrint);
    vm.registerNativeFunction("len", nativeLen);
    vm.registerNativeFunction("range", nativeRange);
    
    // Type checking functions
    vm.registerNativeFunction("type", nativeType);
    vm.registerNativeFunction("isnil", nativeIsNil);
    vm.registerNativeFunction("isboolean", nativeIsBoolean);
    vm.registerNativeFunction("isinteger", nativeIsInteger);
    vm.registerNativeFunction("isfloat", nativeIsFloat);
    vm.registerNativeFunction("isnumber", nativeIsNumber);
    vm.registerNativeFunction("isstring", nativeIsString);
    vm.registerNativeFunction("islist", nativeIsList);
    vm.registerNativeFunction("isdict", nativeIsDict);
    vm.registerNativeFunction("isfunction", nativeIsFunction);
    
    // Math functions
    vm.registerNativeFunction("abs", nativeAbs);
    vm.registerNativeFunction("min", nativeMin);
    vm.registerNativeFunction("max", nativeMax);
    vm.registerNativeFunction("sum", nativeSum);
    
    // String functions
    vm.registerNativeFunction("int", nativeInt);
    vm.registerNativeFunction("str", nativeStr);
    vm.registerNativeFunction("substr", nativeSubstr);
    vm.registerNativeFunction("any", nativeAny);
    vm.registerNativeFunction("all", nativeAll);
    // Iteration helpers
    vm.registerNativeFunction("iter", nativeIter);
    vm.registerNativeFunction("next", nativeNext);
    vm.registerNativeFunction("enumerate", nativeEnumerate);
    vm.registerNativeFunction("zip", nativeZip);
    vm.registerNativeFunction("map", nativeMap);
    vm.registerNativeFunction("filter", nativeFilter);
    
    // List functions
    vm.registerNativeFunction("append", nativeAppend);
    vm.registerNativeFunction("remove", nativeRemove);
    vm.registerNativeFunction("extend", nativeExtend);
    vm.registerNativeFunction("insert", nativeInsert);
    vm.registerNativeFunction("pop", nativePop);
    vm.registerNativeFunction("clear", nativeClear);
    vm.registerNativeFunction("sort", nativeSort);
    vm.registerNativeFunction("reverse", nativeReverse);
    vm.registerNativeFunction("sorted", nativeSorted);
    vm.registerNativeFunction("reversed", nativeReversed);
    vm.registerNativeFunction("count", nativeCount);
    vm.registerNativeFunction("index", nativeIndex);
    vm.registerNativeFunction("list_copy", nativeListCopy);
    
    // Dict functions
    vm.registerNativeFunction("keys", nativeKeys);
    vm.registerNativeFunction("values", nativeValues);
    vm.registerNativeFunction("contains", nativeContains);
    vm.registerNativeFunction("update", nativeUpdate);
    vm.registerNativeFunction("get", nativeGet);
    vm.registerNativeFunction("copy", nativeCopy);
    vm.registerNativeFunction("fromkeys", nativeFromKeys);
    vm.registerNativeFunction("items", nativeItems);
    vm.registerNativeFunction("dict_pop", nativeDictPop);
    vm.registerNativeFunction("popitem", nativePopItem);
    vm.registerNativeFunction("setdefault", nativeSetDefault);

    // Import function
    vm.registerNativeFunction("__import__", nativeImport);
    // Import-all helper
    vm.registerNativeFunction("__import_all__", nativeImportAll);
    // Import bind helper (internal)
    vm.registerNativeFunction("__import_bind__", nativeImportBind);
}

} // namespace rglite
