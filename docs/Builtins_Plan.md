# RGLite 与 Python 内置函数“完全对齐”计划清单

## 计划目标
- 覆盖 Python 内置函数，并保持名称、参数、返回值与异常行为一致。
- 在 RGLite 中统一为“内置函数”，可直接在脚本中调用。
- 对已存在的差异（如 `range` 返回列表而非迭代对象）制定补齐方案。

## 完成标准（每个内置函数必须满足）
- 在 `src/runtime/builtins/BuiltinFunctions.cpp` 实现原生函数。
- 在 `include/BuiltinFunctions.h` 声明原生函数。
- 在 `src/backend/codegen/CodeGenerator.cpp` 的 `isBuiltinFunction` 集合登记。
- 在 `src/frontend/semantic/SemanticAnalyzer.cpp` 的 `defineBuiltins` 注册符号。
 - 在 `tests/unit/test_builtins.cpp` 添加正向与异常路径测试，覆盖边界与类型错误。
 - 在 `docs/Builtins_Plan.md` 更新说明；标注与 Python 的任何细微差异（若存在）。

## 步骤与未对齐清单

### 步骤 1 — 核心与 I/O
- 已对齐：`print`, `len`, `range`
- 未对齐：
  - `input`
  - `open`（按文本模式最小支持；二进制与缓冲可后续扩展）
  - `help`
  - `ascii`
  - `breakpoint`

### 步骤 2 — 迭代与组合
- 已对齐：`any`, `all`, `sum`, `sorted`, `reversed`, `range`, `enumerate`, `zip`, `iter`, `next`, `map`, `filter`
- 备注：
  - `iter` 支持列表、字典（默认 keys）、元组、集合与字符串（逐字符）；
  - `next` 支持默认值参数；当迭代结束时返回默认值或抛出 `StopIteration` 等价异常；
  - `enumerate` 返回形如 `[ [index, item], ... ]` 的列表；
  - `zip` 短路为最短序列长度，返回元素对的列表；
  - `map(func, iterable)` 返回应用函数后的列表；
  - `filter(func, iterable)` 按 `isTruthy` 规则保留元素，返回列表；

### 步骤 3 — 数值与格式
- 已对齐：`abs`, `min`, `max`
- 未对齐：
  - `round`
  - `pow`
  - `divmod`
  - `bin`
  - `hex`
  - `oct`

### 步骤 4 — 类型与转换
- 已对齐：`type`, `int`, `str`
- 未对齐：
  - `bool`
  - `float`
  - `complex`
  - `bytes`
  - `bytearray`
  - `memoryview`
  - `list`
  - `tuple`
  - `dict`
  - `set`
  - `frozenset`
  - `slice`

### 步骤 5 — 反射与环境
- 已对齐：`type`
- 未对齐：
  - `isinstance`
  - `issubclass`
  - `callable`
  - `getattr`
  - `setattr`
  - `hasattr`
  - `delattr`
  - `dir`
  - `vars`
  - `globals`
  - `locals`
  - `id`
  - `hash`
  - `property`
  - `staticmethod`
  - `classmethod`
  - `super`

### 步骤 6 — 字符与字符串
- 已对齐：`str`（另有 `substr`，为 RGLite 专有差异；建议后续引入切片语义）
- 未对齐：
  - `chr`
  - `ord`
  - `format`
  - `repr`

### 步骤 7 — 执行与编译
- 已对齐：`__import__`, `__import_all__`, `__import_bind__`
- 未对齐：
  - `compile`
  - `eval`
  - `exec`

## 实施顺序（建议）
1. 迭代与组合：`enumerate`, `zip`, `iter`, `next`, `map`, `filter`
2. 数值与格式：`round`, `pow`, `divmod`, `bin`, `hex`, `oct`
3. 类型与转换：`bool`, `float`, `list`, `tuple`, `dict`, `set`, `frozenset`, `slice`
4. 反射与环境：`isinstance`, `issubclass`, `callable`, `getattr`/`setattr`/`hasattr`/`delattr`, `dir`, `vars`, `globals`, `locals`, `id`, `hash`, `property`, `staticmethod`, `classmethod`, `super`
5. 字符与字符串：`chr`, `ord`, `format`, `repr`
6. 核心与 I/O：`input`, `open`, `help`, `ascii`, `breakpoint`
7. 执行与编译：`compile`, `eval`, `exec`

## 差异项与补齐策略
### 方法与属性对齐（重要）
- 与 Python 完全对齐：列表与字典的方法仅通过对象方法调用，不再支持函数式全局调用。
- 受影响的方法（仅示例，不限于此）：
  - 列表：`append`, `remove`, `extend`, `insert`, `pop`, `clear`, `sort`, `reverse`, `count`, `index`, `copy`
  - 字典：`keys`, `values`, `items`, `update`, `get`, `pop`, `popitem`, `setdefault`, `copy`, `fromkeys`
- 示例改造：
  - `append(out, x)` → `out.append(x)`
  - `extend(res, left)` → `res.extend(left)`
  - `list_copy(arr)` → `arr.copy()`
  - `keys(d)` → `d.keys()`
  - `dict_pop(d, k)` → `d.pop(k)` / `d.popitem()`

- `range`：当前返回“整数列表”。若需完全对齐 Python 行为，计划新增 `RangeObject` 可迭代类型，支持懒生成、切片与 `len`；保持迭代行为与 CPython 一致。
- 字符串切片：目前提供 `substr(s, start, length)`。为对齐 Python，计划在解析与 VM 层引入切片语义（含负索引与步长），并保留 `substr` 作为兼容扩展。

## 测试与验收
- 为每个新增内置编写单元测试：
  - 正向用例：典型调用、边界值、迭代场景。
  - 异常用例：类型错误、值错误与边界错误，断言异常类型与消息一致。
- 测试位置：`tests/unit/test_builtins.cpp`；命名统一为 `Builtins.<Name>...`。
- 验收标准：构建与测试在 CI 通过，文档更新，并在示例或 `examples/` 中提供至少一个使用示例（如需）。
