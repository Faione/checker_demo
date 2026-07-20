CLANG_ANALYZER ?= clang
CHECKER ?= unix.Malloc
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -g
SOURCE := malloc_leak_path_sensitive.c

.PHONY: analyze clean

analyze:
	$(CLANG_ANALYZER) --analyze $(CFLAGS) \
		-Xclang -analyzer-checker=$(CHECKER) \
		$(SOURCE)

clean:
	rm -rf *.plist *.o *.dSYM
