#include "ctu_chain.h"

/*
 * router 层继续拉长调用链。
 * 每个入口先进入不同 route，再进入 service/gateway/bottom。
 */

int ctu_route_http(int request) { return ctu_service_authorize(request + 100); }

int ctu_route_cron(int job) { return ctu_service_batch(job + 200); }

int ctu_route_cli(int command) { return ctu_service_direct(command + 300); }

int ctu_route_retry(int attempt) {
  return ctu_gateway_emergency(attempt + 400);
}

int ctu_route_healthcheck(int probe) {
  return ctu_service_no_sink(probe + 500);
}
