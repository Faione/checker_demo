#include <stdlib.h>

char *open_resource(void);

struct BufferHolder {
  char *copy_buffer;
};

void fill_by_value(void) {
  struct BufferHolder local = {0};
  local.copy_buffer = open_resource();
}
