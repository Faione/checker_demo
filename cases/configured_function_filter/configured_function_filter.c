#include <stdlib.h>

struct BufferHolder {
  void *tracked_buffer;
  void *ignored_buffer;
};

void fill_buffers(struct BufferHolder *holder) {
  holder->tracked_buffer = malloc(32);
  holder->ignored_buffer = calloc(1, 32);
}
