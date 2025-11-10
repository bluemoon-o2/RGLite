# RGLite 字节码格式说明

本文档说明 RGLite 的字节码格式、变量名表的序列化与在 VM 中的应用。该格式包含“变量名表”用于更好的错误信息和全局变量映射。

## 总览

- 所有数值按小端序（little-endian）编码。
- 头部包含魔术串和版本号：`magic = "RGLB"`，`version`。
- 主体依次是常量池、指令序列，随后是变量名表（如存在）。
- VM 在反序列化后先应用变量名表再进入执行阶段，以保证错误信息中的变量名以及全局映射正确。

## 布局细节

1) Header（头部）
- `uint32`：`magic_len`（魔术串长度，当前为 4）
- `magic_len` 字节：`magic` 内容（固定为 `RGLB`）
- `uint32`：`version`（版本号）

2) Counts（数量信息）
- `uint32`：`const_count`（常量个数）
- `uint32`：`instr_count`（指令条数）

3) Constants（常量池）
- 对每个常量依次写入：
  - `uint8`：类型标签（`ValueType`）
  - 后随具体类型的负载：
    - `BOOLEAN`：`uint8`（0/1）
    - `INTEGER`：`uint64`
    - `FLOAT`：`double`（按位写入的 8 字节）
    - `STRING`：`uint32 len` + `len` 字节内容
    - 容器/函数索引类（`LIST/DICT/TUPLE/SET/FUNCTION/ITERATOR`）：`uint32 index`
    - `NATIVE_FUNCTION`：函数名 `string`（`uint32 len` + `len` 字节）
    - `EXCEPTION`：异常消息 `string`（`uint32 len` + `len` 字节）

4) Instructions（指令序列）
- 每条指令按固定三元组序列化：
  - `uint8`：`opcode`
  - `uint32`：`operand`
  - `uint32`：`line`（源码行号，用于错误定位）

5) Variable Name Table（变量名表）
- 若存在变量名表段：
  - `uint32`：`var_count`（变量名映射条数）
  - 重复 `var_count` 次：
    - `uint32`：`index`（变量索引）
    - `string`：`name`（`uint32 len` + `len` 字节）

VM 在反序列化完成指令后、执行前读取该表并调用 `vm.setVariableName(index, name)` 建立映射。后续异常消息、全局存储等均可利用该映射进行友好展示与访问。

## 示例

以源代码 `x = 5` 为例，字节码的头部与主体大致如下（伪结构）：

```
u32 magic_len = 4
bytes magic = 'R','G','L','B'
u32 version

u32 const_count
u32 instr_count

// const_count 个常量（按类型序列化）
// instr_count 条指令，每条: u8 opcode, u32 operand, u32 line

// 变量名表（如存在）
u32 var_count
repeat var_count times:
  u32 index
  u32 name_len
  bytes name
```

在执行路径中，`Compiler::executeBytecode` 与 `Compiler::executeBytecodeWithVM` 都会在读取完常量与指令后，若存在变量名表，则继续读取并应用，再调用 `vm.interpret(...)`。

## 兼容性与测试

- 读取端将按照头部中的版本号与布局解析；若未包含变量名表段，则直接进入执行。
- `tests/unit/test_bytecode.cpp` 中包含基础字节码生成用例，覆盖算术、条件、循环、列表、成员与索引赋值。

## 参考实现位置

- 写入：`src/RGLite.cpp` 中 `Compiler::compile` 写入头部（魔术串与版本号），并序列化常量、指令与变量名表。
- 读取与应用：`Compiler::executeBytecode` 与 `Compiler::executeBytecodeWithVM` 负责反序列化并在存在变量名表的情况下应用后执行。
