# ctu_reachability_chain

这个 case 用来探索 LLVM 15 Clang Static Analyzer 的 CTU 能力边界：

```text
ctu_chain_demo.c
  -> ctu_chain_entry.c
  -> ctu_chain_router.c
  -> ctu_chain_service.c
  -> ctu_chain_gateway.c
  -> ctu_chain_bottom.c
```

目标底层函数是 `ctu_bottom_sink`。分析入口是 `ctu_chain_demo.c`，CTU index 只包含入口
TU 之外的外部 TU，避免多个 `main` 或重复 USR 同时进入 `externalDefMap.txt`。

普通编译运行：

```sh
make run
```

查看预期入口路径：

```sh
make show-expected
```

运行 CTU reachability 验证：

```sh
make analyze \
  CLANG_ANALYZER=/path/to/llvm-project/build/bin/clang \
  CLANG_EXTDEF_MAPPING=/path/to/llvm-project/build/bin/clang-extdef-mapping
```

如果 CTU 生效，`debug.ExprInspection` 会把 `ctu_bottom_sink` 中的
`clang_analyzer_warnIfReached()` 转换成 `REACHABLE` warning。warning 路径应该从
`ctu_chain_demo.c` 中的入口调用出发，跨多个 TU 到达 `ctu_chain_bottom.c`。
