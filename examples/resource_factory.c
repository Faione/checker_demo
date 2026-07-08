#include "resource_factory.h"

#include <stdlib.h>
#include <string.h>

/*
 * 这个文件故意作为一个独立 translation unit 存在。
 *
 * 对 CTU 来说，member_assignment_demo.c 只看到 resource_factory.h 中的声明；
 * 函数体在 resource_factory.c 中。开启 CTU 后，Clang Static Analyzer 可以通过
 * externalDefMap.txt 找到这里的定义，并在需要时导入另一个 TU 的 AST。
 */

/*
 * 普通跨 TU 函数：主文件会调用 open_resource，并把返回值写入结构体成员。
 * 当前 checker 关注的是调用点和左值成员，因此即使不开 CTU 也能报告调用；
 * 但 CTU 索引会把这个函数名映射到本文件，供后续更复杂的跨过程 checker 使用。
 */
char *open_resource(const char *name) {
  size_t len = strlen(name) + 1;
  char *resource = malloc(len);
  if (resource == NULL) {
    return NULL;
  }

  memcpy(resource, name, len);
  return resource;
}

/*
 * 专门给 CTU case 使用的函数。
 *
 * 它没有内联在主文件里，而是跨 TU 返回一个资源指针。主文件把这个返回值直接赋给
 * holder->ctu_buffer，用来观察 checker 在多文件项目中的调用点输出。
 */
char *create_remote_resource(void) {
  return open_resource("from ctu resource");
}
