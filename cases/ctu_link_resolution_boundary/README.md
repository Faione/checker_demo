# ctu_link_resolution_boundary

这个 case 用来说明 CTU 和真实链接行为之间的边界。

同一个函数 `select_link_value()` 有两个定义：

| 文件 | 定义 |
|---|---|
| `weak_selector.c` | weak symbol，返回 `0` |
| `strong_selector.c` | strong symbol，返回 `7` |

真实链接时，如果同时链接 weak 和 strong 定义，linker 会选择 strong 定义：

```sh
make run
```

预期输出：

```text
selected=7
```

只链接 weak 定义时：

```sh
make run-weak
```

预期输出：

```text
selected=0
```

CTU 分析不会执行真实 linker 的符号解析。它通过 `externalDefMap.txt` 把函数 USR 映射到
某个外部 TU，因此分析结果取决于 index 中选择了哪个定义。

使用 strong 定义作为 CTU 外部定义：

```sh
make analyze-strong \
  CLANG_ANALYZER=/path/to/llvm-project/build/bin/clang \
  CLANG_EXTDEF_MAPPING=/path/to/llvm-project/build/bin/clang-extdef-mapping
```

使用 weak 定义作为 CTU 外部定义：

```sh
make analyze-weak \
  CLANG_ANALYZER=/path/to/llvm-project/build/bin/clang \
  CLANG_EXTDEF_MAPPING=/path/to/llvm-project/build/bin/clang-extdef-mapping
```

`app.c` 中有一个 `clang_analyzer_warnIfReached()` 探针。导入 weak 定义时，
`select_link_value()` 返回 `0`，理论上会到达探针；导入 strong 定义时，返回 `7`，
理论上不会到达探针。

故意把 weak 和 strong 两个定义同时放进 CTU index：

```sh
make analyze-ambiguous \
  CLANG_ANALYZER=/path/to/llvm-project/build/bin/clang \
  CLANG_EXTDEF_MAPPING=/path/to/llvm-project/build/bin/clang-extdef-mapping
```

这个目标用于观察边界行为：同一个 USR 同时出现在多个外部定义中，CTU index 无法表达
“按最终链接命令选择 strong 定义”的语义。也就是说，CTU 是源码级跨 TU 导入机制，
不是链接器模拟器。
