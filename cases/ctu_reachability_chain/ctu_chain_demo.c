#include "ctu_chain.h"

#include <stdio.h>

/*
 * CTU 长调用链入口程序。
 *
 * 这个文件只直接调用入口函数，不直接调用 ctu_bottom_sink。
 * 如果只看当前 TU，很难知道哪些入口最终会汇聚到底层函数。
 */
int main(void) {
  printf("http=%d\n", ctu_entry_http(1));
  printf("cron=%d\n", ctu_entry_cron(2));
  printf("cli=%d\n", ctu_entry_cli(3));
  printf("retry=%d\n", ctu_entry_retry(4));
  printf("healthcheck=%d\n", ctu_entry_healthcheck(5));
  return 0;
}
