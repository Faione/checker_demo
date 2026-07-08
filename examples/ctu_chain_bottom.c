#include "ctu_chain.h"

/*
 * CTU 长调用链 demo 的“最底层函数”。
 *
 * 探索目标：
 *   给定 ctu_bottom_sink，借助 CTU 导入跨 TU 函数体后，反向识别所有最终会走到
 *   这里的入口函数。
 *
 * 这个函数单独放在 ctu_chain_bottom.c，确保入口 TU 不能只靠本地 AST
 * 看到它的函数体。
 */
int ctu_bottom_sink(int value) { return value * 3 + 7; }
