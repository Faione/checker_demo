#include <stdlib.h>

union ResourceSlot {
  void *buffer;
  long raw;
};

void fill_union(union ResourceSlot *slot) {
  slot->buffer = malloc(64);
}
