# union_member_assignment

验证 `demo.CallTargetMemberLValue` 对 union 成员赋值的表现。当前 checker 输出文案统一写作 `assigned struct member`，但底层使用 `RecordDecl`，因此 union 字段也会被识别。

## 目标代码

```c
slot->buffer = malloc(64);
```

## 运行

```sh
make analyze CLANG_ANALYZER=/path/to/llvm-project/build/bin/clang
```

## 预期输出

应出现一条 `demo.CallTargetMemberLValue` warning，包含：

```text
matched configured function call 'malloc'
assigned struct member: ResourceSlot.buffer (via ->)
```
