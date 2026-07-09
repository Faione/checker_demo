# value_member_assignment

验证 `demo.CallTargetMemberLValue` 能识别函数返回值被赋给结构体值对象成员的场景。

## 目标代码

```c
local.copy_buffer = open_resource();
```

## 运行

```sh
make analyze CLANG_ANALYZER=/path/to/llvm-project/build/bin/clang
```

## 预期输出

应出现一条 `demo.CallTargetMemberLValue` warning，包含：

```text
matched configured function call 'open_resource'
assigned struct member: BufferHolder.copy_buffer (via .)
```
