#include "TestFramework.h"
#include "RGLite.h"
#include "VM.h"
#include "ListValue.h"
#include <memory>

using namespace rglite;

// This file constructs Compiler and VM in each test.
// Source snippets for three sorting implementations.
// Selection sort using for-loops, with inline assertion.
static std::string selectionSortSource() {
    return R"(
def selection_sort(arr):
    a = arr.copy()
    n = len(a)
    for i in range(n):
        min_idx = i
        for j in range(n):
            if j > i:
                if a[j] < a[min_idx]:
                    min_idx = j
        tmp = a[i]
        a[i] = a[min_idx]
        a[min_idx] = tmp
    return a

arr_for = [3, 1, 4, 1, 5]
sorted_for = selection_sort(arr_for)

# Check result equals expected; trigger runtime error on mismatch
def check_list_equals(a, b):
    if len(a) != len(b):
        _ = int("x")
        return 0
    i = 0
    while i < len(a):
        if a[i] != b[i]:
            _ = int("x")
            return 0
        i = i + 1
    return 1

_ = check_list_equals(sorted_for, [1, 1, 3, 4, 5])
)";
}

// Recursive quick sort (function name qsort), with inline assertion.
static std::string quickSortSource() {
    return R"(
def qsort(arr):
    n = len(arr)
    if n <= 1:
        return arr.copy()
    pivot = arr[0]
    left = []
    right = []
    i = 1
    while i < n:
        x = arr[i]
        if x <= pivot:
            left.append(x)
        else:
            right.append(x)
        i = i + 1
    left_sorted = qsort(left)
    right_sorted = qsort(right)
    res = []
    res.extend(left_sorted)
    res.append(pivot)
    res.extend(right_sorted)
    return res

arr_rec = [3, 1, 4, 1, 5]
sorted_recursive = qsort(arr_rec)

# Check result equals expected; trigger runtime error on mismatch
def check_list_equals(a, b):
    if len(a) != len(b):
        _ = int("x")
        return 0
    i = 0
    while i < len(a):
        if a[i] != b[i]:
            _ = int("x")
            return 0
        i = i + 1
    return 1

_ = check_list_equals(sorted_recursive, [1, 1, 3, 4, 5])
)";
}

// Counting sort implementation, with inline assertion.
static std::string countingSortSource() {
    return R"(
def counting_sort(arr, max_val):
    counts = []
    i = 0
    while i <= max_val:
        counts.append(0)
        i = i + 1
    i = 0
    while i < len(arr):
        v = arr[i]
        counts[v] = counts[v] + 1
        i = i + 1
    res = []
    i = 0
    while i <= max_val:
        c = counts[i]
        while c > 0:
            res.append(i)
            c = c - 1
        i = i + 1
    return res

arr_dp = [4, 2, 2, 8, 3, 3, 1]
sorted_dp = counting_sort(arr_dp, 8)

# Check result equals expected; trigger runtime error on mismatch
def check_list_equals(a, b):
    if len(a) != len(b):
        _ = int("x")
        return 0
    i = 0
    while i < len(a):
        if a[i] != b[i]:
            _ = int("x")
            return 0
        i = i + 1
    return 1

_ = check_list_equals(sorted_dp, [1, 2, 2, 3, 3, 4, 8])
)";
}

// No C++-side list assertion needed; script raises on mismatch.

TEST(Sorting, ForLoopSelectionSort) {
    auto compiler = createCompiler();
    int ret = compiler->execute(selectionSortSource());
    EXPECT_EQ(ret, 0);
}

TEST(Sorting, RecursiveQuickSort) {
    auto compiler = createCompiler();
    int ret = compiler->execute(quickSortSource());
    EXPECT_EQ(ret, 0);
}

TEST(Sorting, CountingSortDP) {
    auto compiler = createCompiler();
    int ret = compiler->execute(countingSortSource());
    EXPECT_EQ(ret, 0);
}

RUN_ALL_TESTS()
