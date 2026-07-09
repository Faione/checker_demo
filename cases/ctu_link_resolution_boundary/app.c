#include "selector.h"

#include <stdio.h>

void clang_analyzer_warnIfReached(void);

#ifndef __clang_analyzer__
void clang_analyzer_warnIfReached(void) {}
#endif

static int check_selected_value(void) {
  int value = select_link_value();

  if (value == 0) {
    clang_analyzer_warnIfReached();
  }

  return value;
}

int main(void) {
  printf("selected=%d\n", check_selected_value());
  return 0;
}
