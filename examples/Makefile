CC ?= clang
CLANG_ANALYZER ?= clang
CLANG_EXTDEF_MAPPING ?= clang-extdef-mapping
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -g
TARGET := member_assignment_demo
SOURCES := member_assignment_demo.c resource_factory.c
CTU_DIR := ctu
CTU_INDEX := $(CTU_DIR)/externalDefMap.txt

.PHONY: all clean analyze-with-demo-checker ctu-build-index analyze-with-demo-checker-ctu

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $@

analyze-with-demo-checker:
	$(CLANG_ANALYZER) --analyze \
		-Xclang -analyzer-checker=demo.CallTargetMemberLValue \
		-Xclang -analyzer-config \
		-Xclang 'demo.CallTargetMemberLValue:FunctionNames=malloc;open_resource;create_remote_resource' \
		$(SOURCES)

compile_commands.json:
	printf '[\n' > $@
	printf '  {"directory":"%s","command":"%s %s -c member_assignment_demo.c","file":"member_assignment_demo.c"},\n' "$$(pwd)" "$(CC)" "$(CFLAGS)" >> $@
	printf '  {"directory":"%s","command":"%s %s -c resource_factory.c","file":"resource_factory.c"}\n' "$$(pwd)" "$(CC)" "$(CFLAGS)" >> $@
	printf ']\n' >> $@

ctu-build-index: compile_commands.json
	mkdir -p $(CTU_DIR)
	$(CLANG_EXTDEF_MAPPING) -p . $(SOURCES) > $(CTU_INDEX)

analyze-with-demo-checker-ctu: ctu-build-index
	$(CLANG_ANALYZER) --analyze \
		-Xclang -analyzer-checker=demo.CallTargetMemberLValue \
		-Xclang -analyzer-config \
		-Xclang 'demo.CallTargetMemberLValue:FunctionNames=malloc;open_resource;create_remote_resource' \
		-Xclang -analyzer-config \
		-Xclang ctu-dir=$(CTU_DIR) \
		member_assignment_demo.c

clean:
	rm -rf $(TARGET) *.plist *.o compile_commands.json $(CTU_DIR)
