# RGLite 内置函数一览与 Python 对齐说明

本文档列出当前 RGLite 支持的内置函数，并说明其与 Python 内置或标准方法的对应关系与差异。实现以 `src/runtime/builtins/BuiltinFunctions.cpp` 注册为准，并与代码生成器、语义分析器保持一致。

## 已支持的内置

- I/O 与核心：`print`, `len`
- 类型检查：`type`, `isnil`, `isboolean`, `isinteger`, `isfloat`, `isnumber`, `isstring`, `islist`, `isdict`, `isfunction`
- 数学：`abs`, `min`, `max`
- 转换与字符串：`int`, `str`, `substr`
- 迭代与聚合：`any`, `all`, `sum`, `sorted`, `reversed`
- 列表方法：`list.append(x)`, `list.remove(x)`, `list.extend(iterable)`, `list.insert(i, x)`, `list.pop([i])`, `list.clear()`, `list.sort()`, `list.reverse()`, `list.count(x)`, `list.index(x)`, `list.copy()`
- 字典方法：`dict.keys()`, `dict.values()`, `dict.items()`, `dict.update(d2)`, `dict.get(key[, default])`, `dict.pop(key[, default])`, `dict.popitem()`, `dict.setdefault(key[, default])`, `dict.copy()`, `dict.fromkeys(iterable[, value])`, `dict.clear()`

## 与 Python 的关系

- 与 Python 完全一致的名称与语义：`print`, `len`, `abs`, `min`, `max`, `int`, `str`, `type`
- 列表/字典方法：RGLite 现已支持并推荐以对象方法形式调用，语义与 Python 保持一致（如 `list.append(x)`, `dict.keys()` 等）。为兼容旧代码，等价的函数式用法仍可调用，但不再推荐。
- `substr(s, start, length)` 对应 Python 的切片 `s[start:start+length]`；目前以函数形式提供。
- 类型检查函数以 `is*` 系列提供，Python 通常使用 `isinstance(x, T)`, `issubclass(A, B)` 等；RGLite 暂未提供这些，改用细分类型判断函数。

## 对象方法语法

- 访问属性或方法采用 `obj.member`，在调用方法时写作 `obj.member(args...)`。
- 对于 `list` 与 `dict`：当成员名是合法方法名时，成员解析将返回“可调用对象”，随后 `CALL` 会将接收者作为第一个隐式参数传入原生实现，实现与 Python 一致的对象方法调用。
- 字典的成员解析优先返回方法对象而非键值：例如 `{ "keys": 1 }.keys()` 调用的是字典方法，而不是取键名为 `"keys"` 的值。若需按键名访问，请使用索引语法 `d["keys"]`。

## 一致性策略

- 代码生成器在遇到成员调用表达式时，将接收者作为隐式首参数计入 `CALL` 的参数个数，以保证方法调用的栈布局与原生函数约定一致。
- VM 在执行 `GET_ATTR` 时：
  - `list`：若成员为标准列表方法名，返回该方法的可调用对象，并自动捕获接收者；`length` 成员返回长度整数。
  - `dict`：标准字典方法名优先返回方法对象；非方法名按键查找返回对应值。
- 所有方法均通过 `BuiltinFunctions.cpp` 统一注册，保证调用一致性与可维护性。

## 后续规划（建议）

- 引入 Python 风格的别名与方法桥接（如支持 `list.copy`/`dict.popitem` 的方法语法）。
- 补充 Python 内置如 `enumerate`, `zip`, `round` 等，按需实现并保持语义一致。
- 提供 `isinstance`, `issubclass` 的等价接口，以提升类型检查与 Python 对齐度。

如需新增或调整内置，请同步修改：

- `src/runtime/builtins/BuiltinFunctions.cpp`（注册与实现）
- `include/BuiltinFunctions.h`（声明）
- `src/backend/codegen/CodeGenerator.cpp`（`isBuiltinFunction` 集合）
- `src/frontend/semantic/SemanticAnalyzer.cpp`（符号表内置注册）
## 语义补充

- `list.extend(iterable)`：对接收者列表进行就地扩展并返回该列表。支持的可迭代类型包括：`list`、`tuple`、`set`、`string`、`dict`（使用键）。字符串按字符追加（每个字符作为长度为 1 的字符串），字典默认追加其键。如果参数非可迭代对象，抛出 `TypeError`。
- `any(iterable)` / `all(iterable)`：对列表、元组、集合、字典（键）与字符串依次应用真值判断。`any` 只要存在真值元素即为真；`all` 所有元素为真才为真。空字符串在 `any` 为假，在 `all` 为假；字符串按字符视为元素。
- `sum(iterable[, start])`：对数值可迭代（`list`/`tuple`/`set`）求和，可选初始值 `start`。若存在浮点参与，返回浮点；否则返回整数。字符串不可作为数值可迭代，遇到非数值元素抛出 `TypeError`。
- `sorted(iterable, reverse=false)`：返回一个新的列表，对数值与字符串按自然顺序排序；混合类型按内部类型序进行比较。`reverse=true` 时降序。该函数不修改原可迭代对象。
- `reversed(iterable)`：返回一个新的列表，元素顺序反转；字符串按字符反转。不修改原对象。
