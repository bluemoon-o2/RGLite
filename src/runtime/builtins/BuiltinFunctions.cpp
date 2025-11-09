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
                std::cout << list->toString();
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

// Register all builtin functions with the VM
void registerBuiltinFunctions(VM& vm) {
    // I/O functions
    vm.registerNativeFunction("print", nativePrint);
    vm.registerNativeFunction("len", nativeLen);
    
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
}

} // namespace rglite
