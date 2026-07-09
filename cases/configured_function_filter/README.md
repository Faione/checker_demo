# configured_function_filter

验证 `demo.CallTargetMemberLValue` 的 `FunctionNames` 配置是否生效。

## 目标代码

```c
holder->tracked_buffer = malloc(32);
holder->ignored_buffer = calloc(1, 32);
```

## 运行

默认只配置 `malloc`：

```sh
make analyze CLANG_ANALYZER=/path/to/llvm-project/build/bin/clang
```

同时配置 `malloc;calloc`：

```sh
make analyze-with-calloc CLANG_ANALYZER=/path/to/llvm-project/build/bin/clang
```

## 预期输出

默认运行只应报告 `malloc` 对 `BufferHolder.tracked_buffer` 的赋值。

`analyze-with-calloc` 应额外报告：

```text
matched configured function call 'calloc'
assigned struct member: BufferHolder.ignored_buffer (via ->)
```
