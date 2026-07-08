// RUN: %clang_analyze_cc1 -analyzer-checker=demo.CallTargetMemberLValue \
// RUN:   -analyzer-config demo.CallTargetMemberLValue:FunctionNames=malloc,open_resource \
// RUN:   -verify %s

typedef __SIZE_TYPE__ size_t;
void *malloc(size_t size);
void *calloc(size_t count, size_t size);
void *open_resource(void);

struct Holder {
  void *buffer;
  void *resource;
};

void assign_to_member(struct Holder *H) {
  H->buffer = malloc(16); // expected-warning{{matched configured function call 'malloc'.*assigned struct member: Holder.buffer}}

  H->resource = open_resource(); // expected-warning{{matched configured function call 'open_resource'.*assigned struct member: Holder.resource}}
}

void ignored(struct Holder *H) {
  H->buffer = calloc(1, 16);
}
