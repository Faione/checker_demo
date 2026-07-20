#include <stddef.h>

/*
 * These functions intentionally have no definitions in this translation unit.
 * The example verifies that unix.Malloc can model an opaque project API from
 * its ownership attributes alone.
 *
 * In ownership_returns(malloc, 1):
 *   malloc selects the malloc/free resource family (AF_Malloc);
 *   1 selects the first parameter as the allocation size.
 *
 * In ownership_takes(malloc, 1):
 *   malloc selects the same resource family;
 *   1 selects the pointer parameter consumed by the function.
 */
void *project_alloc(size_t size)
    __attribute__((ownership_returns(malloc, 1)));

void project_free(void *ptr)
    __attribute__((ownership_takes(malloc, 1)));

/* Positive case: the annotated allocation remains Allocated when p dies. */
void leak_test(void) {
  void *p = project_alloc(32);
  (void)p;
} // expected: Potential leak of memory pointed to by 'p' [unix.Malloc]

/* Negative case: ownership_takes transitions the symbol to Released. */
void normal_test(void) {
  void *p = project_alloc(32);
  project_free(p);
} // expected: no warning

/* Positive case: the second ownership_takes call sees Released state. */
void double_free_test(void) {
  void *p = project_alloc(32);
  project_free(p);
  project_free(p); // expected: Attempt to free released memory [unix.Malloc]
}
