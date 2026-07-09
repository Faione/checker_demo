#include "ctu_chain.h"

void clang_analyzer_warnIfReached(void);

#ifndef __clang_analyzer__
void clang_analyzer_warnIfReached(void) {}
#endif

/*
 * CTU 长调用链 demo 的“最底层函数”。
 *
 * 探索目标：
 *   给定 ctu_bottom_sink，借助 CTU 导入跨 TU 函数体后，反向识别所有最终会走到
 *   这里的入口函数。
 *
 * 这个函数单独放在 ctu_chain_bottom.c，确保入口 TU 不能只靠本地 AST
 * 看到它的函数体。
 *
 * clang_analyzer_warnIfReached 是 debug.ExprInspection checker 提供的探针。
 * 普通编译时上方提供一个空实现；analyzer 运行时会通过这个探针产生 warning。
 * 这些 warning 落在底层 TU，用来证明 CTU 已经把跨 TU 函数体导入并内联到了
 * 从 main 出发的路径里。
 */
int ctu_bottom_sink(int value, enum CtuEntryKind kind) {
  switch (kind) {
  case CTU_ENTRY_HTTP:
    clang_analyzer_warnIfReached();
    break;
  case CTU_ENTRY_CRON:
    clang_analyzer_warnIfReached();
    break;
  case CTU_ENTRY_CLI:
    clang_analyzer_warnIfReached();
    break;
  case CTU_ENTRY_RETRY:
    clang_analyzer_warnIfReached();
    break;
  }

  return value * 3 + 7;
}
