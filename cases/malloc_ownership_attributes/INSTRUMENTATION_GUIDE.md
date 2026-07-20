# Ownership attribute 最小插桩指南

本文面向 Clang 15.0.4 `unix.Malloc`，用于给实现不可见的项目自定义内存接口补充所有权契约。

## 1. 定义兼容宏

建议在公共 annotation 头文件中集中定义：

```c
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#if defined(__clang__) && __has_attribute(ownership_returns)

#define CSA_RETURNS_MALLOC \
  __attribute__((ownership_returns(malloc)))

#define CSA_RETURNS_MALLOC_SIZE(index) \
  __attribute__((ownership_returns(malloc, index)))

#define CSA_TAKES_MALLOC(index) \
  __attribute__((ownership_takes(malloc, index)))

#define CSA_HOLDS_MALLOC(index) \
  __attribute__((ownership_holds(malloc, index)))

#else

#define CSA_RETURNS_MALLOC
#define CSA_RETURNS_MALLOC_SIZE(index)
#define CSA_TAKES_MALLOC(index)
#define CSA_HOLDS_MALLOC(index)

#endif
```

非 Clang 编译器看不到这些契约，但仍能正常编译项目。

## 2. 标注接口声明

```c
#include <stddef.h>

void *project_alloc(size_t size)
    CSA_RETURNS_MALLOC_SIZE(1);

void project_free(void *ptr)
    CSA_TAKES_MALLOC(1);

void project_enqueue(void *ptr)
    CSA_HOLDS_MALLOC(1);
```

优先标注调用方可见的公共声明，而不是只标注 `.c/.cpp` 文件中的定义。

## 3. 参数含义

| 属性 | 含义 |
|---|---|
| `ownership_returns(malloc)` | 返回一个属于 `AF_Malloc` 的资源；分配大小未知 |
| `ownership_returns(malloc, N)` | 返回 `AF_Malloc` 资源，并用第 N 个整数参数作为分配大小 |
| `ownership_takes(malloc, N)` | 消耗并释放第 N 个指针参数，状态转为 `Released` |
| `ownership_holds(malloc, N)` | 接管第 N 个指针参数的释放责任，状态转为 `Relinquished` |

其中 `malloc` 是资源协议标识符：

- 不是字符串；
- 不是对标准库 `malloc()` 的调用；
- 在 Clang 15.0.4 `MallocChecker` 中映射到 `AF_Malloc`；
- 使分配结果与 `free` 类释放操作属于同一分配族。

## 4. 参数索引规则

C 函数和 C++ 静态成员函数从 1 开始计算：

```c
void release_second(int tag, void *ptr)
    CSA_TAKES_MALLOC(2);
```

这里 `2` 选择 `ptr`，不是字节数或调用次数。

C++ 非静态成员函数需要计入隐式 `this`，第一个显式参数通常使用索引 2：

```cpp
struct Allocator {
  void *allocate(size_t size)
      CSA_RETURNS_MALLOC_SIZE(2);

  void release(void *ptr)
      CSA_TAKES_MALLOC(2);
};
```

类型约束：

- `ownership_returns(malloc, N)` 的第 N 个参数必须是整数类型；
- `ownership_takes/holds(malloc, N)` 的第 N 个参数必须是指针或 block pointer；
- `takes/holds` 可以列出多个参数序号；
- 同一个参数不能同时标注为 `takes` 和 `holds`。

## 5. 启用 Analyzer 配置

Clang 15.0.4 需要显式启用：

```text
unix.DynamicMemoryModeling:Optimistic=true
```

Clang Driver 示例：

```sh
clang --analyze source.c \
  -Xclang -analyzer-checker=core,unix.Malloc \
  -Xclang -analyzer-config \
  -Xclang unix.DynamicMemoryModeling:Optimistic=true
```

本 case 可直接运行：

```sh
make analyze CLANG_ANALYZER=/path/to/clang
```

生产和 CI 中应显式传入该选项，不依赖不同 Clang 发行版的默认行为。

## 6. 最小验证

插桩后至少验证以下三种情况：

```c
void leak_test(void) {
  void *p = project_alloc(32);
  (void)p;
} // 应报告 Potential leak

void normal_test(void) {
  void *p = project_alloc(32);
  project_free(p);
} // 不应报告

void double_free_test(void) {
  void *p = project_alloc(32);
  project_free(p);
  project_free(p);
} // 应报告 Attempt to free released memory
```

预期总计两条 `unix.Malloc` 告警：一条泄漏和一条重复释放。

## 7. 已知边界

- Clang 15.0.4 的 `MallocChecker` 只处理 resource module 名为 `malloc` 的 ownership attribute。
- 所有使用 `malloc` 标识符的自定义接口都归入同一个 `AF_Malloc`，不能验证多组 allocator/free 的严格一一配对。
- `ownership_returns` 最多指定一个大小参数，不能直接表示 `count * element_size`。
- `realloc` 的成功、失败和旧指针所有权语义不能只靠 `ownership_returns` 完整表达。
- 如果 CSA 成功内联函数实现，通常会直接分析函数体；attribute 主要用于实现不可见或未内联的接口。
- `ownership_holds` 表示释放责任转移，不表示资源已经被释放。

完整可运行示例见 `malloc_ownership_attributes.c`。
