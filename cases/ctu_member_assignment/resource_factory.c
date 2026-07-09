#include "resource_factory.h"

#include <stdlib.h>
#include <string.h>

char *create_remote_resource(void) {
  const char *text = "from ctu resource";
  size_t len = strlen(text) + 1;
  char *resource = malloc(len);
  if (resource == NULL)
    return NULL;

  memcpy(resource, text, len);
  return resource;
}
