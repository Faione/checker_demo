#include "ctu_chain.h"

/*
 * 入口层。
 *
 * 这些函数模拟外部可见入口。CTU 边界探索的目标是：
 *   从最底层 ctu_bottom_sink 反向找到所有会走到它的入口。
 *
 * 预期会到达 ctu_bottom_sink 的入口：
 *   ctu_entry_http
 *   ctu_entry_cron
 *   ctu_entry_cli
 *   ctu_entry_retry
 *
 * 预期不会到达 ctu_bottom_sink 的入口：
 *   ctu_entry_healthcheck
 */

int ctu_entry_http(int seed) { return ctu_route_http(seed); }

int ctu_entry_cron(int seed) { return ctu_route_cron(seed); }

int ctu_entry_cli(int seed) { return ctu_route_cli(seed); }

int ctu_entry_retry(int seed) { return ctu_route_retry(seed); }

int ctu_entry_healthcheck(int seed) { return ctu_route_healthcheck(seed); }
