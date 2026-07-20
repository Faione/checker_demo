# malloc_ownership_attributes

这个 case 验证 Clang 15.0.4 的 `unix.Malloc` 能否通过 ownership
attributes 建模实现不可见的项目自定义分配/释放接口。

独立的最小插桩步骤见 [`INSTRUMENTATION_GUIDE.md`](INSTRUMENTATION_GUIDE.md)。

## 被标注的接口

```c
void *project_alloc(size_t size)
    __attribute__((ownership_returns(malloc, 1)));

void project_free(void *ptr)
    __attribute__((ownership_takes(malloc, 1)));
```

属性参数含义：

| 属性参数 | 含义 |
|---|---|
| `malloc` | 资源协议标识符，将资源映射到 `AF_Malloc`；不是对标准库 `malloc()` 的调用 |
| `ownership_returns` 中的 `1` | 第一个参数 `size` 是分配大小，CSA 用它建立 DynamicExtent |
| `ownership_takes` 中的 `1` | 第一个参数 `ptr` 被函数消耗并释放，状态转为 `Released` |

两个函数故意只有声明、没有定义。这样可以确认告警来自 attribute contract，
而不是 analyzer 内联函数体后观察到的真实分配或释放操作。

## 必需配置

`unix.Malloc` 默认不会根据 ownership attributes 建模用户自定义函数，必须开启：

```text
unix.DynamicMemoryModeling:Optimistic=true
```

运行：

```sh
make analyze CLANG_ANALYZER=/path/to/llvm-project/build/bin/clang
```

等价的核心命令是：

```sh
clang --analyze -std=c11 \
  -Xclang -analyzer-checker=core,unix.Malloc \
  -Xclang -analyzer-config \
  -Xclang unix.DynamicMemoryModeling:Optimistic=true \
  malloc_ownership_attributes.c
```

## 预期结果

| 函数 | 预期结果 | 原因 |
|---|---|---|
| `leak_test()` | 1 条 `Potential leak ... [unix.Malloc]` | `project_alloc()` 将返回符号设为 `Allocated(AF_Malloc)`，函数结束时没有释放 |
| `normal_test()` | 无告警 | `project_free()` 的 `ownership_takes` 将符号设为 `Released` |
| `double_free_test()` | 1 条 `Attempt to free released memory [unix.Malloc]` | 第二次调用 `project_free()` 时符号已经是 `Released` |

总计预期出现两条 `unix.Malloc` 告警：一条泄漏、一条重复释放。

示例输出：

```text
malloc_ownership_attributes.c:26:1: warning: Potential leak of memory pointed to by 'p' [unix.Malloc]
malloc_ownership_attributes.c:38:3: warning: Attempt to free released memory [unix.Malloc]
2 warnings generated.
```

告警行号可能随注释调整而变化，应以函数名、告警类型和 checker 名称为验证依据。

## 对照实验

不显式传入 `Optimistic`：

```sh
make analyze-without-contract \
  CLANG_ANALYZER=/path/to/llvm-project/build/bin/clang
```

对于本调研目标 Clang 15.0.4，`Checkers.td` 中 `Optimistic` 的默认值是 `false`，
因此预期上述两个 opaque 函数不再由 ownership contract 建模，泄漏和 double-free
告警都不出现。

不过该行为不能作为跨版本断言：发行版可能调整默认值、选项成熟度或 ownership
attribute 的启用方式。例如本 case 创建时使用的 Apple Clang 21.0.0 在没有显式配置时
仍然产生相同的两条告警。因此生产和 CI 命令应始终显式写出
`unix.DynamicMemoryModeling:Optimistic=true`，并使用目标编译器版本验证。

## 扩展到 ownership_holds

需要表达“函数接管资源，但调用后指针仍可能被使用”时，可以声明：

```c
void project_enqueue(void *ptr)
    __attribute__((ownership_holds(malloc, 1)));
```

调用后，`unix.Malloc` 将资源状态转为 `Relinquished(AF_Malloc)`，调用方不再承担
释放责任。此时再由调用方执行 `free(ptr)`，可能报告释放非自有内存。

## 插桩注意事项

- attribute 应放在调用方可见的公共声明中，通常是项目头文件。
- `malloc` 是当前 `MallocChecker` 实际支持的 resource module 名称。
- C 函数参数序号从 1 开始；C++ 非静态成员函数还要考虑隐式 `this`。
- `ownership_returns(malloc, N)` 的第 N 个参数必须是整数类型。
- `ownership_takes/holds(malloc, N)` 的第 N 个参数必须是指针或 block pointer。
- 这些属性只能表达 `AF_Malloc` 分配族，不能区分多组 allocator/free 的严格一一配对。
- `realloc` 的成功/失败所有权语义不能只靠 `ownership_returns` 完整表达。
