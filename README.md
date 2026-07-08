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
examples/resource_factory.c
examples/resource_factory.h
examples/ctu_chain_*.c
examples/ctu_chain.h
examples/ctu_chain_expected_paths.txt
tests/call-target-member-lvalue.c
patches/register-in-clang-15.0.4.patch
```

| 文件 | 作用 |
|---|---|
| `src/CallTargetMemberLValueChecker.cpp` | checker 实现源码 |
| `examples/member_assignment_demo.c` | 可普通编译运行的主示例 C 程序 |
| `examples/resource_factory.c` | CTU case 使用的另一个 translation unit |
| `examples/resource_factory.h` | 跨 TU 函数声明 |
| `examples/ctu_chain_*.c` | CTU 能力边界探索用的长调用链 demo |
| `examples/ctu_chain_expected_paths.txt` | `ctu_bottom_sink` 的预期入口和调用链 |
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
holder->ctu_buffer = create_remote_resource();
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
member_assignment_demo.c:...: warning: matched configured function call 'malloc'; callee declaration: ...; call site: ...; assigned struct member: BufferHolder.heap_buffer (via ->) [demo.CallTargetMemberLValue]
member_assignment_demo.c:...: warning: matched configured function call 'open_resource'; callee declaration: resource_factory.h:...; call site: ...; assigned struct member: BufferHolder.copy_buffer (via .) [demo.CallTargetMemberLValue]
member_assignment_demo.c:...: warning: matched configured function call 'create_remote_resource'; callee declaration: resource_factory.h:...; call site: ...; assigned struct member: BufferHolder.ctu_buffer (via ->) [demo.CallTargetMemberLValue]
```

## CTU 示例

`examples/resource_factory.c` 和 `examples/member_assignment_demo.c` 共同组成一个
Cross Translation Unit（CTU）学习样例：

```text
member_assignment_demo.c
  -> include resource_factory.h
  -> call create_remote_resource()

resource_factory.c
  -> define create_remote_resource()
  -> define open_resource()
```

普通编译会同时编译两个 `.c` 文件：

```sh
cd examples
make
```

如果要运行 CTU 分析，需要同时提供已经构建好的 `clang` 和
`clang-extdef-mapping`：

```sh
cd examples
make analyze-with-demo-checker-ctu \
  CLANG_ANALYZER=/path/to/llvm-project/build/bin/clang \
  CLANG_EXTDEF_MAPPING=/path/to/llvm-project/build/bin/clang-extdef-mapping
```

该目标会生成：

```text
compile_commands.json
ctu/externalDefMap.txt
```

并把：

```text
-analyzer-config ctu-dir=ctu
```

传给 analyzer。

注意：当前 checker 主要演示“调用点 + 左值成员识别”，本身不需要读取被调函数体也能报告
`holder->ctu_buffer = create_remote_resource()`。CTU case 的意义是把示例项目扩展成真实的多
translation unit 形态，方便后续继续学习“导入另一个 TU 的函数体并做跨过程推理”的 checker。

## CTU 能力边界探索：长调用链

`examples/ctu_chain_*.c` 提供了一个专门用于探索 CTU 能力边界的长调用链 demo。

目标底层函数是：

```text
ctu_bottom_sink
```

它定义在：

```text
ctu_chain_bottom.c
```

入口函数定义在：

```text
ctu_chain_entry.c
```

中间经过多个 translation unit：

```text
ctu_chain_entry.c
  -> ctu_chain_router.c
  -> ctu_chain_service.c
  -> ctu_chain_gateway.c
  -> ctu_chain_bottom.c
```

普通运行：

```sh
cd examples
make ctu-chain
./ctu_chain_demo
```

查看预期路径：

```sh
make ctu-chain-show-expected
```

预期会走到 `ctu_bottom_sink` 的入口：

```text
ctu_entry_http
ctu_entry_cron
ctu_entry_cli
ctu_entry_retry
```

预期不会走到 `ctu_bottom_sink` 的入口：

```text
ctu_entry_healthcheck
```

生成 CTU 索引：

```sh
make ctu-build-index \
  CLANG_ANALYZER=/path/to/llvm-project/build/bin/clang \
  CLANG_EXTDEF_MAPPING=/path/to/llvm-project/build/bin/clang-extdef-mapping
```

该命令会基于 `compile_commands.json` 生成 LLVM 15 CTU 需要的两个输入文件：

```text
ctu/externalDefMap.txt
ctu/invocations.yaml
```

其中：

| 文件 | 作用 |
|---|---|
| `ctu/externalDefMap.txt` | 由 `clang-extdef-mapping` 生成，描述函数 USR 到外部 TU 源文件的映射 |
| `ctu/invocations.yaml` | 描述每个外部 TU 应该如何被 clang 重新解析；LLVM 15 的 on-demand CTU 需要它 |

验证 CTU 是否真的导入并分析了跨 TU 函数体：

```sh
make analyze-ctu-chain-reachability \
  CLANG_ANALYZER=/path/to/llvm-project/build/bin/clang \
  CLANG_EXTDEF_MAPPING=/path/to/llvm-project/build/bin/clang-extdef-mapping
```

这个目标使用：

```text
-analyzer-checker=debug.ExprInspection
-analyzer-config experimental-enable-naive-ctu-analysis=true
-analyzer-config ctu-dir=ctu
-analyzer-config ctu-invocation-list=ctu/invocations.yaml
-analyzer-config ctu-phase1-inlining=all
-analyzer-config display-ctu-progress=true
```

`ctu_bottom_sink` 中放置了 `clang_analyzer_warnIfReached()` 探针。普通编译时它是空函数；
analyzer 运行时由 `debug.ExprInspection` 把它转换成 warning。

如果 CTU 生效，预期会在 `ctu_chain_bottom.c` 中看到多条 `REACHABLE` warning，
并且 `display-ctu-progress=true` 会打印外部 AST 加载信息。每条 warning 的路径应该从
`ctu_chain_demo.c` 中的入口调用出发，跨过 `entry/router/service/gateway` 等不同 TU，
最终到达 `ctu_bottom_sink`。

注意：只使用 `debug.DumpCallGraph` 不足以验证这个能力。它主要打印当前 AST 的调用图；
在 LLVM 15 中，如果没有显式开启：

```text
experimental-enable-naive-ctu-analysis=true
ctu-invocation-list=ctu/invocations.yaml
```

analyzer 通常只会分析主 TU 中可见的函数，看起来就像“只分析到了 main 中的函数”。
这个 demo 改用 warning 管线来证明跨 TU 函数体确实被导入和内联。

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
