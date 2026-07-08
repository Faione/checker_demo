# checker_demos

本目录是一个 Clang Static Analyzer checker 示例工程。

checker 名称为：

```text
demo.CallTargetMemberLValue
```

它演示了三个能力：

1. 通过配置输入一组函数名，例如 `malloc;calloc`。
2. 匹配对这些函数的调用，并打印被调函数声明位置。
3. 如果调用结果被赋值给结构体或联合体成员，则在 analyzer warning 中给出结构体名称和成员名称。

## 文件说明

```text
src/CallTargetMemberLValueChecker.cpp
examples/member_assignment_demo.c
tests/call-target-member-lvalue.c
patches/register-in-clang-15.0.4.patch
```

| 文件 | 作用 |
|---|---|
| `src/CallTargetMemberLValueChecker.cpp` | checker 实现源码 |
| `examples/member_assignment_demo.c` | 可普通编译运行的示例 C 程序 |
| `examples/Makefile` | 示例程序构建与 checker 分析命令 |
| `tests/call-target-member-lvalue.c` | lit 风格测试样例 |
| `patches/register-in-clang-15.0.4.patch` | 将 checker 注册进 Clang 15.0.4 源码树的参考补丁 |

## 安装到 Clang 源码树

假设 LLVM 源码目录为：

```text
/path/to/llvm-project
```

先复制 checker 源码：

```sh
cp src/CallTargetMemberLValueChecker.cpp \
  /path/to/llvm-project/clang/lib/StaticAnalyzer/Checkers/
```

然后手动注册 checker，或参考这个补丁：

```text
patches/register-in-clang-15.0.4.patch
```

补丁内容很小，主要修改两个位置：

```text
clang/include/clang/StaticAnalyzer/Checkers/Checkers.td
clang/lib/StaticAnalyzer/Checkers/CMakeLists.txt
```

如果你的 Clang 源码树已经有其它本地改动，补丁上下文可能需要手动调整。

## 编译

在 LLVM build 目录中执行：

```sh
ninja clang
```

## 运行

示例命令：

```sh
/path/to/llvm-project/build/bin/clang \
  --analyze \
  -Xclang -analyzer-checker=demo.CallTargetMemberLValue \
  -Xclang -analyzer-config \
  -Xclang 'demo.CallTargetMemberLValue:FunctionNames=malloc;open_resource' \
  sample.c
```

checker 会通过 Clang Static Analyzer 的 warning 管线输出结果，例如：

```text
sample.c:10:19: warning: matched configured function call 'malloc'; callee declaration: sample.c:2:7; call site: sample.c:10:19; assigned struct member: Holder.buffer (via ->) [demo.CallTargetMemberLValue]
```

## 可编译示例

`examples/member_assignment_demo.c` 是一个普通 C 程序，可以先直接编译运行：

```sh
cd examples
make
./member_assignment_demo
```

这个示例包含两类目标写法：

```c
holder->heap_buffer = malloc(32);
local.copy_buffer = open_resource("from open_resource");
```

安装并编译好自定义 checker 后，可以对同一个示例运行分析：

```sh
cd examples
make analyze-with-demo-checker \
  CLANG_ANALYZER=/path/to/llvm-project/build/bin/clang
```

注意：`-analyzer-config` 本身使用逗号分隔多个配置项，因此命令行里不要写成
`FunctionNames=malloc,open_resource`。这里使用分号分隔函数名，并用引号保护整个
配置参数，避免 shell 把分号解释成命令分隔符。

预期会看到类似输出：

```text
member_assignment_demo.c:23:25: warning: matched configured function call 'malloc'; callee declaration: member_assignment_demo.c:12:22; call site: member_assignment_demo.c:23:25; assigned struct member: BufferHolder.heap_buffer (via ->) [demo.CallTargetMemberLValue]
member_assignment_demo.c:33:23: warning: matched configured function call 'open_resource'; callee declaration: member_assignment_demo.c:9:14; call site: member_assignment_demo.c:33:23; assigned struct member: BufferHolder.copy_buffer (via .) [demo.CallTargetMemberLValue]
```

## 测试样例

`tests/call-target-member-lvalue.c` 是一个 lit 风格测试。安装 checker 后，可以把它复制到：

```text
clang/test/Analysis/call-target-member-lvalue.c
```

然后执行：

```sh
ninja check-clang-analysis
```

## 当前实现范围

当前实现支持直接函数调用和简单赋值形式：

```c
holder.buffer = malloc(16);
holder->buffer = malloc(16);
```

它会识别左值中的 `MemberExpr`，并在 warning 消息中输出：

```text
结构体名称.成员名称
```

当前没有处理以下情况：

| 情况 | 说明 |
|---|---|
| 函数指针间接调用 | 例如 `fp()` |
| 先赋值给临时变量再写入成员 | 例如 `p = malloc(16); holder.buffer = p;` |
| 复杂数据流追踪 | 该 demo 只展示 AST 结构匹配和 CSA checker 接入方式 |

## 说明

这是一个 demo checker，但它已经使用 `PathSensitiveBugReport` 进入 Clang Static Analyzer 的标准 warning 管线，而不是直接写 `stderr`。因此同一份结果可以被文本输出、plist、IDE 或 CI 报告继续消费。
