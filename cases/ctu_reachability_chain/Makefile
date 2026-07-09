CC ?= clang
CLANG_ANALYZER ?= clang
CLANG_EXTDEF_MAPPING ?= clang-extdef-mapping
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -g
TARGET := ctu_chain_demo
SOURCES := \
	ctu_chain_demo.c \
	ctu_chain_entry.c \
	ctu_chain_router.c \
	ctu_chain_service.c \
	ctu_chain_gateway.c \
	ctu_chain_bottom.c
CTU_EXTERNAL_SOURCES := \
	ctu_chain_entry.c \
	ctu_chain_router.c \
	ctu_chain_service.c \
	ctu_chain_gateway.c \
	ctu_chain_bottom.c
CTU_DIR := ctu
CTU_INDEX := $(CTU_DIR)/externalDefMap.txt
CTU_INVOCATIONS := $(CTU_DIR)/invocations.yaml

.PHONY: all run show-expected analyze analyze-ctu ctu-build-index ctu-build-invocations clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $@

run: $(TARGET)
	./$(TARGET)

show-expected:
	cat ctu_chain_expected_paths.txt

compile_commands.json:
	printf '[\n' > $@
	first=1; \
	for src in $(SOURCES); do \
		if [ $$first -eq 0 ]; then printf ',\n' >> $@; fi; \
		first=0; \
		printf '  {"directory":"%s","command":"%s %s -c %s","file":"%s"}' "$$(pwd)" "$(CC)" "$(CFLAGS)" "$$src" "$$src" >> $@; \
	done
	printf '\n]\n' >> $@

ctu-build-invocations:
	mkdir -p $(CTU_DIR)
	rm -f $(CTU_INVOCATIONS)
	for src in $(SOURCES); do \
		abs="$$(pwd)/$$src"; \
		printf '"%s":\n' "$$abs" >> $(CTU_INVOCATIONS); \
		printf '  - "%s"\n' "$(CLANG_ANALYZER)" >> $(CTU_INVOCATIONS); \
		printf '  - "-std=c11"\n' >> $(CTU_INVOCATIONS); \
		printf '  - "-c"\n' >> $(CTU_INVOCATIONS); \
		printf '  - "%s"\n' "$$abs" >> $(CTU_INVOCATIONS); \
	done

ctu-build-index: compile_commands.json ctu-build-invocations
	mkdir -p $(CTU_DIR)
	$(CLANG_EXTDEF_MAPPING) -p . $(CTU_EXTERNAL_SOURCES) > $(CTU_INDEX)

analyze analyze-ctu: ctu-build-index
	$(CLANG_ANALYZER) --analyze $(CFLAGS) \
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
	rm -rf $(TARGET) *.dSYM *.plist *.o compile_commands.json $(CTU_DIR)
