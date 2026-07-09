CC ?= clang
CLANG_ANALYZER ?= clang
CLANG_EXTDEF_MAPPING ?= clang-extdef-mapping
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -g
CTU_DIR := ctu
CTU_INDEX := $(CTU_DIR)/externalDefMap.txt
CTU_INVOCATIONS := $(CTU_DIR)/invocations.yaml
SOURCES := app.c weak_selector.c strong_selector.c

.PHONY: all run run-weak run-strong run-linker-choice analyze analyze-weak analyze-strong analyze-ambiguous ctu-build-invocations ctu-index-weak ctu-index-strong ctu-index-ambiguous clean

all: linker_choice_demo weak_demo strong_demo

linker_choice_demo: app.c weak_selector.c strong_selector.c selector.h
	$(CC) $(CFLAGS) app.c weak_selector.c strong_selector.c -o $@

weak_demo: app.c weak_selector.c selector.h
	$(CC) $(CFLAGS) app.c weak_selector.c -o $@

strong_demo: app.c strong_selector.c selector.h
	$(CC) $(CFLAGS) app.c strong_selector.c -o $@

run run-linker-choice: linker_choice_demo
	./linker_choice_demo

run-weak: weak_demo
	./weak_demo

run-strong: strong_demo
	./strong_demo

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

ctu-index-weak: compile_commands.json ctu-build-invocations
	mkdir -p $(CTU_DIR)
	$(CLANG_EXTDEF_MAPPING) -p . weak_selector.c > $(CTU_INDEX)

ctu-index-strong: compile_commands.json ctu-build-invocations
	mkdir -p $(CTU_DIR)
	$(CLANG_EXTDEF_MAPPING) -p . strong_selector.c > $(CTU_INDEX)

ctu-index-ambiguous: compile_commands.json ctu-build-invocations
	mkdir -p $(CTU_DIR)
	$(CLANG_EXTDEF_MAPPING) -p . weak_selector.c strong_selector.c > $(CTU_INDEX)

analyze analyze-strong: ctu-index-strong
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
		app.c

analyze-weak: ctu-index-weak
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
		app.c

analyze-ambiguous: ctu-index-ambiguous
	$(CLANG_ANALYZER) --analyze $(CFLAGS) \
		-Xclang -analyzer-checker=debug.ExprInspection \
		-Xclang -analyzer-config \
		-Xclang experimental-enable-naive-ctu-analysis=true \
		-Xclang -analyzer-config \
		-Xclang ctu-dir=$(CTU_DIR) \
		-Xclang -analyzer-config \
		-Xclang ctu-invocation-list=$(CTU_INVOCATIONS) \
		app.c

clean:
	rm -rf linker_choice_demo weak_demo strong_demo *.dSYM *.plist *.o compile_commands.json $(CTU_DIR)
