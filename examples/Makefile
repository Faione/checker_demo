CC ?= clang
CLANG_ANALYZER ?= clang
CLANG_EXTDEF_MAPPING ?= clang-extdef-mapping
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -g
TARGET := member_assignment_demo
SOURCES := member_assignment_demo.c resource_factory.c
MEMBER_CTU_EXTERNAL_SOURCES := resource_factory.c
CTU_CHAIN_TARGET := ctu_chain_demo
CTU_CHAIN_SOURCES := \
	ctu_chain_demo.c \
	ctu_chain_entry.c \
	ctu_chain_router.c \
	ctu_chain_service.c \
	ctu_chain_gateway.c \
	ctu_chain_bottom.c
CTU_CHAIN_EXTERNAL_SOURCES := \
	ctu_chain_entry.c \
	ctu_chain_router.c \
	ctu_chain_service.c \
	ctu_chain_gateway.c \
	ctu_chain_bottom.c
ALL_SOURCES := $(SOURCES) $(CTU_CHAIN_SOURCES)
CTU_DIR := ctu
CTU_INDEX := $(CTU_DIR)/externalDefMap.txt
CTU_INVOCATIONS := $(CTU_DIR)/invocations.yaml

.PHONY: all clean analyze-with-demo-checker ctu-build-member-index ctu-build-chain-index ctu-build-invocations analyze-with-demo-checker-ctu ctu-chain ctu-chain-show-expected analyze-ctu-chain-reachability

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

ctu-build-invocations:
	mkdir -p $(CTU_DIR)
	rm -f $(CTU_INVOCATIONS)
	for src in $(ALL_SOURCES); do \
		abs="$$(pwd)/$$src"; \
		printf '"%s":\n' "$$abs" >> $(CTU_INVOCATIONS); \
		printf '  - "%s"\n' "$(CLANG_ANALYZER)" >> $(CTU_INVOCATIONS); \
		printf '  - "-std=c11"\n' >> $(CTU_INVOCATIONS); \
		printf '  - "-c"\n' >> $(CTU_INVOCATIONS); \
		printf '  - "%s"\n' "$$abs" >> $(CTU_INVOCATIONS); \
	done

ctu-build-member-index: compile_commands.json ctu-build-invocations
	mkdir -p $(CTU_DIR)
	$(CLANG_EXTDEF_MAPPING) -p . $(MEMBER_CTU_EXTERNAL_SOURCES) > $(CTU_INDEX)

ctu-build-chain-index: compile_commands.json ctu-build-invocations
	mkdir -p $(CTU_DIR)
	$(CLANG_EXTDEF_MAPPING) -p . $(CTU_CHAIN_EXTERNAL_SOURCES) > $(CTU_INDEX)

analyze-with-demo-checker-ctu: ctu-build-member-index
	$(CLANG_ANALYZER) --analyze \
		-Xclang -analyzer-checker=demo.CallTargetMemberLValue \
		-Xclang -analyzer-config \
		-Xclang 'demo.CallTargetMemberLValue:FunctionNames=malloc;open_resource;create_remote_resource' \
		-Xclang -analyzer-config \
		-Xclang experimental-enable-naive-ctu-analysis=true \
		-Xclang -analyzer-config \
		-Xclang ctu-dir=$(CTU_DIR) \
		-Xclang -analyzer-config \
		-Xclang ctu-invocation-list=$(CTU_INVOCATIONS) \
		member_assignment_demo.c

analyze-ctu-chain-reachability: ctu-build-chain-index
	$(CLANG_ANALYZER) --analyze \
		-Xclang -analyzer-checker=debug.ExprInspection \
		-Xclang -analyzer-output=text \
		-Xclang -analyzer-config \
		-Xclang experimental-enable-naive-ctu-analysis=true \
		-Xclang -analyzer-config \
		-Xclang ctu-dir=$(CTU_DIR) \
		-Xclang -analyzer-config \
		-Xclang ctu-invocation-list=$(CTU_INVOCATIONS) \
		-Xclang -analyzer-config \
		-Xclang ctu-phase1-inlining=all \
		-Xclang -analyzer-config \
		-Xclang display-ctu-progress=true \
		ctu_chain_demo.c

clean:
	rm -rf $(TARGET) $(CTU_CHAIN_TARGET) *.dSYM *.plist *.o compile_commands.json $(CTU_DIR)
