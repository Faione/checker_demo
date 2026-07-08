CC ?= clang
CLANG_ANALYZER ?= clang
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -g
TARGET := member_assignment_demo
SOURCES := member_assignment_demo.c

.PHONY: all clean analyze-with-demo-checker

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $< -o $@

analyze-with-demo-checker:
	$(CLANG_ANALYZER) --analyze \
		-Xclang -analyzer-checker=demo.CallTargetMemberLValue \
		-Xclang -analyzer-config \
		-Xclang 'demo.CallTargetMemberLValue:FunctionNames=malloc;open_resource' \
		$(SOURCES)

clean:
	rm -f $(TARGET) *.plist
