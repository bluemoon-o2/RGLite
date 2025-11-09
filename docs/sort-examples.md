# 排序示例使用指南

本目录提供多种排序算法的 RGen 示例脚本，位于 `examples/sort/`：

- `merge_sort.rgb`：归并排序
- `quick_sort.rgb`：快速排序（分区 + 递归）
- `insertion_sort.rgb`：插入排序（逐步有序插入）
- `selection_sort.rgb`：选择排序（逐次选择最小）

所有脚本使用相同示例数据：`[5, 2, 8, 1, 3, 7, 4, 6]`，运行后会输出排序结果。

## 运行示例

在构建目录执行：

```
rg.exe ..\examples\sort\merge_sort.rgb
rg.exe ..\examples\sort\quick_sort.rgb
rg.exe ..\examples\sort\insertion_sort.rgb
rg.exe ..\examples\sort\selection_sort.rgb
```

如需查看编译/执行阶段的简要调试信息，添加 `-d`：

```
rg.exe ..\examples\sort\quick_sort.rgb -d
```

预期输出（任一脚本）：

```
[1, 2, 3, 4, 5, 6, 7, 8]
```

