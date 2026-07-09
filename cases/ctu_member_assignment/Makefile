CC ?= clang
CLANG_ANALYZER ?= clang
CLANG_EXTDEF_MAPPING ?= clang-extdef-mapping
CHECKER ?= demo.CallTargetMemberLValue
FUNCTION_NAMES ?= create_remote_resource
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -g
SOURCES := ctu_member_assignment.c resource_factory.c
OBJECTS := $(SOURCES:.c=.o)
CTU_DIR := ctu
CTU_INVOCATIONS := $(CTU_DIR)/invocations.yaml
CTU_INDEX := $(CTU_DIR)/externalDefMap.txt

.PHONY: all analyze analyze-ctu compile_commands.json ctu-build-index ctu-build-invocations clean

all: $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

analyze:
	$(CLANG_ANALYZER) --analyze $(CFLAGS) \
		-Xclang -analyzer-checker=$(CHECKER) \
		-Xclang -analyzer-config \
		-Xclang '$(CHECKER):FunctionNames=$(FUNCTION_NAMES)' \
		$(SOURCES)

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
	$(CLANG_EXTDEF_MAPPING) -p . resource_factory.c > $(CTU_INDEX)

analyze-ctu: ctu-build-index
	$(CLANG_ANALYZER) --analyze $(CFLAGS) \
		-Xclang -analyzer-checker=$(CHECKER) \
		-Xclang -analyzer-config \
		-Xclang '$(CHECKER):FunctionNames=$(FUNCTION_NAMES)' \
		-Xclang -analyzer-config \
		-Xclang experimental-enable-naive-ctu-analysis=true \
		-Xclang -analyzer-config \
		-Xclang ctu-dir=$(CTU_DIR) \
		-Xclang -analyzer-config \
		-Xclang ctu-invocation-list=$(CTU_INVOCATIONS) \
		ctu_member_assignment.c

clean:
	rm -rf *.dSYM *.plist *.o compile_commands.json $(CTU_DIR)
