#!/bin/sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

CASES="
pointer_member_assignment
value_member_assignment
configured_function_filter
malloc_leak_path_sensitive
malloc_leak_special_contract
malloc_ownership_attributes
union_member_assignment
ctu_member_assignment
ctu_reachability_chain
ctu_link_resolution_boundary
"

usage() {
  cat <<'EOF'
Usage:
  ./cases/run_cases.sh list
  ./cases/run_cases.sh analyze [case]
  ./cases/run_cases.sh build [case]
  ./cases/run_cases.sh run [case]
  ./cases/run_cases.sh clean [case]

Environment:
  CLANG_ANALYZER=/path/to/llvm-project/build/bin/clang
  CLANG_EXTDEF_MAPPING=/path/to/llvm-project/build/bin/clang-extdef-mapping

Notes:
  analyze runs the analyzer-oriented target for each case.
  build compiles cases that have a normal binary/object target.
  run executes cases that have a runnable target.
EOF
}

case_exists() {
  selected=$1
  for case_name in $CASES; do
    if [ "$case_name" = "$selected" ]; then
      return 0
    fi
  done
  return 1
}

target_for() {
  action=$1
  case_name=$2

  case "$action:$case_name" in
    analyze:ctu_member_assignment) printf '%s\n' "analyze-ctu" ;;
    analyze:ctu_reachability_chain) printf '%s\n' "analyze" ;;
    analyze:ctu_link_resolution_boundary) printf '%s\n' "analyze" ;;
    build:ctu_member_assignment) printf '%s\n' "all" ;;
    build:ctu_reachability_chain) printf '%s\n' "all" ;;
    build:ctu_link_resolution_boundary) printf '%s\n' "all" ;;
    run:ctu_reachability_chain) printf '%s\n' "run" ;;
    run:ctu_link_resolution_boundary) printf '%s\n' "run" ;;
    run:*) printf '%s\n' "__skip__" ;;
    clean:*) printf '%s\n' "clean" ;;
    analyze:*) printf '%s\n' "analyze" ;;
    build:*) printf '%s\n' "__skip__" ;;
    *) printf '%s\n' "__unknown__" ;;
  esac
}

run_one() {
  action=$1
  case_name=$2
  target=$(target_for "$action" "$case_name")

  if [ "$target" = "__skip__" ]; then
    printf '[skip] %s: no %s target\n' "$case_name" "$action"
    return 0
  fi

  if [ "$target" = "__unknown__" ]; then
    printf 'unknown action: %s\n' "$action" >&2
    usage >&2
    return 2
  fi

  printf '\n==> %s: make %s\n' "$case_name" "$target"
  make -C "$ROOT_DIR/$case_name" "$target"
}

list_cases() {
  for case_name in $CASES; do
    printf '%s\n' "$case_name"
  done
}

main() {
  action=${1:-}
  selected=${2:-}

  case "$action" in
    list)
      list_cases
      ;;
    analyze|build|run|clean)
      if [ -n "$selected" ]; then
        if ! case_exists "$selected"; then
          printf 'unknown case: %s\n' "$selected" >&2
          usage >&2
          return 2
        fi
        run_one "$action" "$selected"
      else
        for case_name in $CASES; do
          run_one "$action" "$case_name"
        done
      fi
      ;;
    *)
      usage >&2
      return 2
      ;;
  esac
}

main "$@"
