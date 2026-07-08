#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct BufferHolder {
  char *heap_buffer;
  char *copy_buffer;
};

static char *open_resource(const char *name) {
  size_t len = strlen(name) + 1;
  char *resource = malloc(len);
  if (resource == NULL) {
    return NULL;
  }

  memcpy(resource, name, len);
  return resource;
}

static void fill_by_pointer(struct BufferHolder *holder) {
  holder->heap_buffer = malloc(32);
  if (holder->heap_buffer != NULL) {
    snprintf(holder->heap_buffer, 32, "from malloc");
  }
}

static void fill_by_value(struct BufferHolder *holder) {
  struct BufferHolder local = {0};

  local.copy_buffer = open_resource("from open_resource");

  free(holder->copy_buffer);
  holder->copy_buffer = local.copy_buffer;
}

int main(void) {
  struct BufferHolder holder = {0};

  fill_by_pointer(&holder);
  fill_by_value(&holder);

  printf("%s\n", holder.heap_buffer);
  printf("%s\n", holder.copy_buffer);

  free(holder.heap_buffer);
  free(holder.copy_buffer);
  return 0;
}
