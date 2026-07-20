# malloc_leak_path_sensitive

这个 case 用于观察内存泄漏 checker 的路径敏感能力。

`no_leak_by_path_split()` 改写自示例代码：

```c
void no_leak_by_path_split(int b) {
  void *a = malloc(16);
  int c = 0;

  if (b > 0) {
    free(a);
  } else {
    c = -1;
  }

  if (c == -1) {
    free(a);
  }
}
```

虽然释放点分布在两个不同的 `if` 中，但两条路径都会释放 `a`：

| 路径 | 行为 |
|---|---|
| `b > 0` | 第一个分支释放 `a`，`c == -1` 为假 |
| `b <= 0` | `c` 被设为 `-1`，第二个分支释放 `a` |

因此路径敏感的内存泄漏 checker 不应该在这个函数上报告泄漏。

同一文件中还提供了 `leak_when_zero()` 作为对照：当 `b == 0` 时，`a` 没有释放，应该触发
`unix.Malloc` 的泄漏报告。

运行：

```sh
make analyze CLANG_ANALYZER=/path/to/llvm-project/build/bin/clang
```
