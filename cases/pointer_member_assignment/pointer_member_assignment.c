#include <stdlib.h>

struct BufferHolder {
  char *heap_buffer;
};

void fill_by_pointer(struct BufferHolder *holder) {
  holder->heap_buffer = malloc(32);
}
