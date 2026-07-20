#include <stdlib.h>

/*
 * 这个函数改写自问题中的示例。
 *
 * 路径 1：
 *   b > 0 -> free(a) -> c 仍为 0 -> 第二个 if 不执行。
 *
 * 路径 2：
 *   b <= 0 -> c = -1 -> 第二个 if 执行 -> free(a)。
 *
 * 两条路径都会释放 a，因此路径敏感的内存泄漏 checker 不应该在这里报告泄漏。
 */
void no_leak_by_path_split(int b) {
  void *a = malloc(16);
  int c = 0;

  if (b > 0) {
    free(a);
  } else {
    c = -1;
  }

  if (c == -1) {
    free(a);
  }
}

/*
 * 对照样例：当 b == 0 时，a 没有被释放。
 * 该函数用于确认内存泄漏 checker 确实能在同一个 case 中报告真实泄漏。
 */
void leak_when_zero(int b) {
  void *a = malloc(16);

  if (b > 0) {
    free(a);
  }
}
