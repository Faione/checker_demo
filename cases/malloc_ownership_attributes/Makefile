CLANG_ANALYZER ?= clang
CHECKER ?= core,unix.Malloc
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -g
ANALYZER_CONFIG ?= unix.DynamicMemoryModeling:Optimistic=true
SOURCE := malloc_ownership_attributes.c

.PHONY: analyze analyze-without-contract clean

analyze:
	$(CLANG_ANALYZER) --analyze $(CFLAGS) \
		-Xclang -analyzer-checker=$(CHECKER) \
		-Xclang -analyzer-config \
		-Xclang $(ANALYZER_CONFIG) \
		$(SOURCE)

# 对照实验：不显式传入 Optimistic。Clang 15.0.4 源码中的默认值是
# false；较新发行版可能改变选项暴露方式或默认行为，具体结果见 README。
analyze-without-contract:
	$(CLANG_ANALYZER) --analyze $(CFLAGS) \
		-Xclang -analyzer-checker=$(CHECKER) \
		$(SOURCE)

clean:
	rm -rf *.plist *.o *.dSYM
