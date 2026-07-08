CC ?= clang
CLANG_ANALYZER ?= clang
CLANG_EXTDEF_MAPPING ?= clang-extdef-mapping
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -g
TARGET := member_assignment_demo
SOURCES := member_assignment_demo.c resource_factory.c
CTU_CHAIN_TARGET := ctu_chain_demo
CTU_CHAIN_SOURCES := \
	ctu_chain_demo.c \
	ctu_chain_entry.c \
	ctu_chain_router.c \
	ctu_chain_service.c \
	ctu_chain_gateway.c \
	ctu_chain_bottom.c
ALL_SOURCES := $(SOURCES) $(CTU_CHAIN_SOURCES)
CTU_DIR := ctu
CTU_INDEX := $(CTU_DIR)/externalDefMap.txt

.PHONY: all clean analyze-with-demo-checker ctu-build-index analyze-with-demo-checker-ctu ctu-chain ctu-chain-show-expected analyze-ctu-chain-callgraph

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $@

$(CTU_CHAIN_TARGET): $(CTU_CHAIN_SOURCES)
	$(CC) $(CFLAGS) $(CTU_CHAIN_SOURCES) -o $@

ctu-chain: $(CTU_CHAIN_TARGET)

ctu-chain-show-expected:
	cat ctu_chain_expected_paths.txt

analyze-with-demo-checker:
	$(CLANG_ANALYZER) --analyze \
		-Xclang -analyzer-checker=demo.CallTargetMemberLValue \
		-Xclang -analyzer-config \
		-Xclang 'demo.CallTargetMemberLValue:FunctionNames=malloc;open_resource;create_remote_resource' \
		$(SOURCES)

compile_commands.json:
	printf '[\n' > $@
	first=1; \
	for src in $(ALL_SOURCES); do \
		if [ $$first -eq 0 ]; then printf ',\n' >> $@; fi; \
		first=0; \
		printf '  {"directory":"%s","command":"%s %s -c %s","file":"%s"}' "$$(pwd)" "$(CC)" "$(CFLAGS)" "$$src" "$$src" >> $@; \
	done
	printf '\n' >> $@
	printf ']\n' >> $@

ctu-build-index: compile_commands.json
	mkdir -p $(CTU_DIR)
	$(CLANG_EXTDEF_MAPPING) -p . $(ALL_SOURCES) > $(CTU_INDEX)

analyze-with-demo-checker-ctu: ctu-build-index
	$(CLANG_ANALYZER) --analyze \
		-Xclang -analyzer-checker=demo.CallTargetMemberLValue \
		-Xclang -analyzer-config \
		-Xclang 'demo.CallTargetMemberLValue:FunctionNames=malloc;open_resource;create_remote_resource' \
		-Xclang -analyzer-config \
		-Xclang ctu-dir=$(CTU_DIR) \
		member_assignment_demo.c

analyze-ctu-chain-callgraph: ctu-build-index
	$(CLANG_ANALYZER) --analyze \
		-Xclang -analyzer-checker=debug.DumpCallGraph \
		-Xclang -analyzer-config \
		-Xclang ctu-dir=$(CTU_DIR) \
		ctu_chain_demo.c

clean:
	rm -rf $(TARGET) $(CTU_CHAIN_TARGET) *.dSYM *.plist *.o compile_commands.json $(CTU_DIR)
