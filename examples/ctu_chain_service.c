#include "ctu_chain.h"

/*
 * service 层位于另一个 translation unit。
 * 它既包含会到达 ctu_bottom_sink 的函数，也包含不会到达 sink 的对照函数。
 */

int ctu_service_authorize(int token) {
  return ctu_gateway_commit(token + 10, CTU_ENTRY_HTTP);
}

int ctu_service_batch(int job) {
  return ctu_gateway_commit(job + 20, CTU_ENTRY_CRON);
}

int ctu_service_direct(int command) {
  return ctu_bottom_sink(command + 30, CTU_ENTRY_CLI);
}

int ctu_service_no_sink(int probe) { return probe - 1; }
