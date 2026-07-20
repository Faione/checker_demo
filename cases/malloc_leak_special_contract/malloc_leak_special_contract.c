#include <stdlib.h>

enum {
  SPECIAL_KIND = 0xC0DE,
  TRANSFER_PHASE = 7
};

/*
 * Positive case.
 *
 * The local convention is:
 *   kind == SPECIAL_KIND && phase == TRANSFER_PHASE
 * means the temporary buffer must either be registered for later ownership
 * transfer or be freed before returning.
 *
 * This function forgets to do either action only on that exact path.
 */
void leak_only_on_special_contract(int kind, int phase) {
  void *payload = malloc(64);

  if (kind == SPECIAL_KIND) {
    if (phase == TRANSFER_PHASE) {
      return;
    }
  }

  free(payload);
}

/*
 * Negative case 1.
 *
 * The same special path is reachable, but the convention is satisfied by
 * freeing the temporary buffer before returning.
 */
void no_leak_when_special_contract_is_satisfied(int kind, int phase) {
  void *payload = malloc(64);

  if (kind == SPECIAL_KIND) {
    if (phase == TRANSFER_PHASE) {
      free(payload);
      return;
    }
  }

  free(payload);
}

/*
 * Negative case 2.
 *
 * Syntactically, there is a return without free inside the nested branch.
 * Semantically, that branch is unreachable because the outer guard already
 * established kind != SPECIAL_KIND.
 */
void no_leak_when_special_contract_is_infeasible(int kind, int phase) {
  void *payload = malloc(64);

  if (kind != SPECIAL_KIND) {
    if (kind == SPECIAL_KIND && phase == TRANSFER_PHASE) {
      return;
    }
  }

  free(payload);
}
