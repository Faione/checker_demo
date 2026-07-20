# malloc_leak_special_contract

这个 case 用于验证 `unix.Malloc` 在“特殊约定条件”下的路径敏感泄漏检测能力。

本地约定：

```c
kind == SPECIAL_KIND && phase == TRANSFER_PHASE
```

表示临时 buffer 必须在返回前被释放，或者被注册给后续所有权转移流程。

## 函数说明

| 函数 | 预期 |
|---|---|
| `leak_only_on_special_contract()` | 仅在 `kind == 0xC0DE && phase == 7` 的路径上泄漏 |
| `no_leak_when_special_contract_is_satisfied()` | 同样的特殊路径可达，但返回前释放，不应报告泄漏 |
| `no_leak_when_special_contract_is_infeasible()` | 源码中存在 `return` without `free`，但该路径被矛盾约束剪枝，不应报告泄漏 |

## 验证点

这个例子同时验证三件事：

1. Analyzer 会把 `kind == SPECIAL_KIND` 和 `phase == TRANSFER_PHASE` 分裂成独立路径。
2. `MallocChecker` 会在每条路径上分别维护 `payload` 的 `Allocated` / `Released` 状态。
3. 约束矛盾的路径不会被当成真实泄漏路径。

运行：

```sh
make analyze CLANG_ANALYZER=/path/to/llvm-project/build/bin/clang
```

预期只出现一条 `unix.Malloc` 泄漏报告，报告上下文为 `leak_only_on_special_contract`。
