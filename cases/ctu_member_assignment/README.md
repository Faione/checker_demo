# ctu_member_assignment

验证 `demo.CallTargetMemberLValue` 在多 translation unit 项目中的表现。

当前 checker 关注的是调用点和赋值左值，因此即使不开 CTU，也能在入口文件中识别：

```c
holder->ctu_buffer = create_remote_resource();
```

`analyze-ctu` 目标额外生成 CTU 索引，供后续扩展 checker 时验证跨 TU 函数体导入。

## 运行

普通构建：

```sh
make
```

该目标只编译对象文件；这个 case 是 analyzer 输入，不提供 `main`。

运行 checker：

```sh
make analyze CLANG_ANALYZER=/path/to/llvm-project/build/bin/clang
```

运行 CTU 版本：

```sh
make analyze-ctu \
  CLANG_ANALYZER=/path/to/llvm-project/build/bin/clang \
  CLANG_EXTDEF_MAPPING=/path/to/llvm-project/build/bin/clang-extdef-mapping
```

## 预期输出

应出现一条 `demo.CallTargetMemberLValue` warning，包含：

```text
matched configured function call 'create_remote_resource'
assigned struct member: BufferHolder.ctu_buffer (via ->)
```
