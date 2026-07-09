CLANG_ANALYZER ?= clang
CHECKER ?= demo.CallTargetMemberLValue
FUNCTION_NAMES ?= malloc
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -g
SOURCE := configured_function_filter.c

.PHONY: analyze analyze-with-calloc clean

analyze:
	$(CLANG_ANALYZER) --analyze $(CFLAGS) \
		-Xclang -analyzer-checker=$(CHECKER) \
		-Xclang -analyzer-config \
		-Xclang '$(CHECKER):FunctionNames=$(FUNCTION_NAMES)' \
		$(SOURCE)

analyze-with-calloc:
	$(MAKE) analyze FUNCTION_NAMES='malloc;calloc'

clean:
	rm -rf *.plist *.o *.dSYM
