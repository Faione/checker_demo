# pointer_member_assignment

验证 `demo.CallTargetMemberLValue` 能识别函数返回值被赋给结构体指针成员的场景。

## 目标代码

```c
holder->heap_buffer = malloc(32);
```

## 运行

```sh
make analyze CLANG_ANALYZER=/path/to/llvm-project/build/bin/clang
```

## 预期输出

应出现一条 `demo.CallTargetMemberLValue` warning，包含：

```text
matched configured function call 'malloc'
assigned struct member: BufferHolder.heap_buffer (via ->)
```
