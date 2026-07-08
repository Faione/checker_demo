#include "ctu_chain.h"

/*
 * gateway 层负责把 service 层请求汇聚到底层 sink。
 * 这里故意保留两个不同 wrapper，模拟大型项目中多个模块最终汇聚到同一底层函数。
 */

int ctu_gateway_commit(int payload, enum CtuEntryKind kind) {
  return ctu_bottom_sink(payload + 1, kind);
}

int ctu_gateway_emergency(int payload, enum CtuEntryKind kind) {
  return ctu_bottom_sink(payload + 2, kind);
}
