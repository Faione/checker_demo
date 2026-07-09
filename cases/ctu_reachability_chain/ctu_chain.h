#ifndef CTU_CHAIN_H
#define CTU_CHAIN_H

int ctu_entry_http(int seed);
int ctu_entry_cron(int seed);
int ctu_entry_cli(int seed);
int ctu_entry_retry(int seed);
int ctu_entry_healthcheck(int seed);

enum CtuEntryKind {
  CTU_ENTRY_HTTP = 1,
  CTU_ENTRY_CRON = 2,
  CTU_ENTRY_CLI = 3,
  CTU_ENTRY_RETRY = 4
};

int ctu_route_http(int request);
int ctu_route_cron(int job);
int ctu_route_cli(int command);
int ctu_route_retry(int attempt);
int ctu_route_healthcheck(int probe);

int ctu_service_authorize(int token);
int ctu_service_batch(int job);
int ctu_service_direct(int command);
int ctu_service_no_sink(int probe);

int ctu_gateway_commit(int payload, enum CtuEntryKind kind);
int ctu_gateway_emergency(int payload, enum CtuEntryKind kind);

int ctu_bottom_sink(int value, enum CtuEntryKind kind);

#endif
