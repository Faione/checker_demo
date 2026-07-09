#include "resource_factory.h"

struct BufferHolder {
  char *ctu_buffer;
};

void fill_by_ctu_resource(struct BufferHolder *holder) {
  holder->ctu_buffer = create_remote_resource();
}
