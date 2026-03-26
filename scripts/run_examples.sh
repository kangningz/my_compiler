#!/usr/bin/env bash

set +e

BUILD_DIR="build"
COMPILER="./${BUILD_DIR}/my_compiler"

echo "=============================="
echo "Running MiniC example tests..."
echo "=============================="

run_test() {
    local source_file=$1
    local output_ll=$2
    local expected=$3

    echo
    echo "[TEST] ${source_file}"

    ${COMPILER} "${source_file}" --emit-llvm -o "${BUILD_DIR}/${output_ll}"
    if [ $? -ne 0 ]; then
        echo "[FAIL] failed to generate LLVM IR for ${source_file}"
        exit 1
    fi

    lli-20 "${BUILD_DIR}/${output_ll}"
    local actual=$?

    echo "Expected: ${expected}"
    echo "Actual  : ${actual}"

    if [ "${actual}" -eq "${expected}" ]; then
        echo "[PASS] ${source_file}"
    else
        echo "[FAIL] ${source_file}"
        exit 1
    fi
}

run_test "examples/add.mc" "add.ll" 7
run_test "examples/if_test.mc" "if_test.ll" 13
run_test "examples/while_test.mc" "while_test.ll" 5

echo
echo "=============================="
echo "All example tests passed."
echo "=============================="