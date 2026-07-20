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
cases/*/
cases/run_cases.sh
tests/call-target-member-lvalue.c
patches/register-in-clang-15.0.4.patch
```

| 文件 | 作用 |
|---|---|
| `src/CallTargetMemberLValueChecker.cpp` | checker 实现源码 |
| `cases/*/` | 按 checker 能力拆分的独立验证用例，每个 case 自带 Makefile 和 README |
| `cases/run_cases.sh` | 统一列出、分析、构建、运行或清理 case 的辅助脚本 |
| `tests/call-target-member-lvalue.c` | lit 风格测试样例 |
| `patches/register-in-clang-15.0.4.patch` | 将 checker 注册进 Clang 15.0.4 源码树的参考补丁 |

## 独立能力验证用例

`cases/` 下的每个目录都是独立 case：

| Case | 验证点 |
|---|---|
| `cases/pointer_member_assignment` | `holder->field = malloc(...)`，识别 `via ->` |
| `cases/value_member_assignment` | `local.field = open_resource()`，识别 `via .` |
| `cases/configured_function_filter` | `FunctionNames` 只报告配置过的函数 |
| `cases/malloc_leak_path_sensitive` | 内存泄漏 checker 的路径敏感释放判断 |
| `cases/malloc_leak_special_contract` | 仅在特殊约定条件下触发的路径敏感内存泄漏 |
| `cases/malloc_ownership_attributes` | 使用 `ownership_returns/takes` 建模实现不可见的自定义分配和释放接口 |
| `cases/union_member_assignment` | union 字段也能通过 `RecordDecl` 被识别 |
| `cases/ctu_member_assignment` | 多 translation unit 项目中的成员赋值调用点 |
| `cases/ctu_reachability_chain` | 长调用链跨 TU reachability，验证 CTU 能否导入并到达底层函数 |
| `cases/ctu_link_resolution_boundary` | weak/strong 同名符号的链接选择，说明 CTU 不模拟 linker 解析 |

列出所有 case：

```sh
./cases/run_cases.sh list
```

运行单个 case 的 analyzer 目标：

```sh
./cases/run_cases.sh analyze pointer_member_assignment
```

运行所有 case 的 analyzer 目标：

```sh
CLANG_ANALYZER=/path/to/llvm-project/build/bin/clang \
CLANG_EXTDEF_MAPPING=/path/to/llvm-project/build/bin/clang-extdef-mapping \
./cases/run_cases.sh analyze
```

普通构建或执行可运行 case：

```sh
./cases/run_cases.sh build ctu_reachability_chain
./cases/run_cases.sh run ctu_reachability_chain
```

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

## Case 说明

每个 case 都可以进入子目录后直接运行 `make analyze`。CTU case 还会生成
`compile_commands.json`、`ctu/externalDefMap.txt` 和 `ctu/invocations.yaml`。

`cases/ctu_member_assignment` 演示多 TU 项目中的成员赋值调用点。当前 checker 主要关注
“调用点 + 左值成员识别”，所以即使不开 CTU，也能在直接调用点报告
`holder->ctu_buffer = create_remote_resource()`；CTU 配置用于学习外部 TU 函数体导入流程。

`cases/ctu_reachability_chain` 演示更长的跨 TU 调用链：

```text
ctu_chain_demo.c
  -> ctu_chain_entry.c
  -> ctu_chain_router.c
  -> ctu_chain_service.c
  -> ctu_chain_gateway.c
  -> ctu_chain_bottom.c
```

目标底层函数是 `ctu_bottom_sink`。该 case 使用 `debug.ExprInspection` 的
`clang_analyzer_warnIfReached()` 探针验证 analyzer 是否真的导入并内联了跨 TU 函数体。
如果 CTU 生效，预期会在 `ctu_chain_bottom.c` 中看到多条 `REACHABLE` warning。

注意：LLVM 15 的 on-demand CTU 需要显式传入：

```text
experimental-enable-naive-ctu-analysis=true
ctu-dir=ctu
ctu-invocation-list=ctu/invocations.yaml
```

`externalDefMap.txt` 只应该包含当前分析入口 TU 之外的外部定义。如果把多个独立程序的入口
文件一起传给 `clang-extdef-mapping`，例如同时包含两个 `main`，LLVM 15 会报：

```text
multiple definitions are found for the same key in index
```

`cases/ctu_link_resolution_boundary` 演示与链接行为相关的边界：同一个函数同时存在 weak
定义和 strong 定义时，真实链接会选择 strong 定义；CTU 则只按照 `externalDefMap.txt`
导入某个源码定义，不能表达“根据最终链接命令选择哪个符号定义”的语义。如果把两个定义都放入
CTU index，同一个 USR 会变成歧义映射。

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
