#include "resource_factory.h"

#include <stdio.h>
#include <stdlib.h>

/*
 * 这个文件是 CallTargetMemberLValueChecker 的学习样例。
 *
 * 运行普通程序时，它只是一个简单的 C 程序：
 *   make
 *   ./member_assignment_demo
 *
 * 使用自定义 checker 分析时，Makefile 会传入：
 *   demo.CallTargetMemberLValue:FunctionNames=malloc;open_resource;create_remote_resource
 *
 * 因此 checker 只关注对 malloc、open_resource 和 create_remote_resource
 * 的直接调用。
 * 如果这些调用的返回值被赋给结构体成员，checker 会通过 analyzer warning
 * 输出“函数位置 + 调用点位置 + 结构体成员名称”。
 */

/* checker 需要在左值中看到结构体成员，例如 holder->heap_buffer 或 local.copy_buffer。 */
struct BufferHolder {
  char *heap_buffer;
  char *copy_buffer;
  char *ctu_buffer;
  char *ignored_buffer;
};

/*
 * open_resource 在 resource_factory.c 中定义。
 * 这里通过 resource_factory.h 只看到它的声明，用来形成一个跨 TU 调用。
 */

/*
 * 示例 1：通过指针访问结构体成员。
 *
 * 这里的左值是 holder->heap_buffer，AST 中对应 MemberExpr，并且 isArrow()
 * 为 true。checker 会输出：
 *   assigned struct member: BufferHolder.heap_buffer (via ->)
 */
static void fill_by_pointer(struct BufferHolder *holder) {
  holder->heap_buffer = malloc(32);
  if (holder->heap_buffer != NULL) {
    snprintf(holder->heap_buffer, 32, "from malloc");
  }
}

/*
 * 示例 2：通过对象值访问结构体成员。
 *
 * local.copy_buffer 使用点号访问成员，AST 中仍然是 MemberExpr，但 isArrow()
 * 为 false。checker 会输出：
 *   assigned struct member: BufferHolder.copy_buffer (via .)
 */
static void fill_by_value(struct BufferHolder *holder) {
  struct BufferHolder local = {0};

  local.copy_buffer = open_resource("from open_resource");

  free(holder->copy_buffer);
  holder->copy_buffer = local.copy_buffer;
}

/*
 * 示例 3：CTU case。
 *
 * create_remote_resource 在 resource_factory.c 中定义，不在当前文件中。
 * 普通 AST 里，当前 TU 只能看到 header 声明；开启 CTU 后，Analyzer 可以通过
 * externalDefMap.txt 找到 resource_factory.c 中的函数定义。
 *
 * 当前 checker 的核心逻辑仍然是“直接调用 + 左值成员识别”，所以 warning 会落在
 * 这里的调用点，并输出：
 *   assigned struct member: BufferHolder.ctu_buffer (via ->)
 */
static void fill_by_ctu_resource(struct BufferHolder *holder) {
  holder->ctu_buffer = create_remote_resource();
}

/*
 * 对照样例：这个赋值形态也是“函数返回值写入结构体成员”，但当前 Makefile
 * 没有把 calloc 放进 FunctionNames 配置，所以 checker 不会报告这一行。
 *
 * 如果把配置改成：
 *   FunctionNames=malloc;open_resource;create_remote_resource;calloc
 * 那么这一行也会产生 warning，并输出 BufferHolder.ignored_buffer。
 */
static void fill_ignored_call(struct BufferHolder *holder) {
  holder->ignored_buffer = calloc(1, 8);
  if (holder->ignored_buffer != NULL) {
    snprintf(holder->ignored_buffer, 8, "ignored");
  }
}

int main(void) {
  struct BufferHolder holder = {0};

  fill_by_pointer(&holder);
  fill_by_value(&holder);
  fill_by_ctu_resource(&holder);
  fill_ignored_call(&holder);

  printf("%s\n", holder.heap_buffer);
  printf("%s\n", holder.copy_buffer);
  printf("%s\n", holder.ctu_buffer);
  printf("%s\n", holder.ignored_buffer);

  free(holder.heap_buffer);
  free(holder.copy_buffer);
  free(holder.ctu_buffer);
  free(holder.ignored_buffer);
  return 0;
}
