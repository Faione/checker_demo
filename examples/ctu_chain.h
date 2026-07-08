#ifndef CTU_CHAIN_H
#define CTU_CHAIN_H

int ctu_entry_http(int seed);
int ctu_entry_cron(int seed);
int ctu_entry_cli(int seed);
int ctu_entry_retry(int seed);
int ctu_entry_healthcheck(int seed);

int ctu_route_http(int request);
int ctu_route_cron(int job);
int ctu_route_cli(int command);
int ctu_route_retry(int attempt);
int ctu_route_healthcheck(int probe);

int ctu_service_authorize(int token);
int ctu_service_batch(int job);
int ctu_service_direct(int command);
int ctu_service_no_sink(int probe);

int ctu_gateway_commit(int payload);
int ctu_gateway_emergency(int payload);

int ctu_bottom_sink(int value);

#endif
