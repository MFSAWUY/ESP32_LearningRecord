#!/usr/bin/env bash
# Build only: uses the separately installed Linux packages; never flashes.
set -euo pipefail
handbook_root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
test_root=${HANDBOOK_TEST_ROOT:-"$(dirname -- "$handbook_root")/test"}
export IDF_PATH="$test_root/idf-platformio/packages/framework-espidf"
export IDF_TOOLS_PATH="$test_root/idf-platformio"
export IDF_PYTHON_ENV_PATH="$test_root/idf-venv"
export ESP_IDF_VERSION=6.1.0
export IDF_PYTHON_CHECK_CONSTRAINTS=no
export PYTHONUTF8=1
export IDF_PY_BUILD_JOBS=4
export ESP_ROM_ELF_DIR="$IDF_TOOLS_PATH/packages/tool-esp-rom-elfs/"
export PATH="$IDF_TOOLS_PATH/packages/toolchain-xtensa-esp-elf/bin:$IDF_PYTHON_ENV_PATH/bin:$PATH"
python_bin="$IDF_PYTHON_ENV_PATH/bin/python"
[[ -x "$python_bin" && -f "$IDF_PATH/tools/idf.py" ]] || { echo 'IDF packages / Python environment are missing.' >&2; exit 2; }
"$python_bin" -c 'import json,sys; from pathlib import Path; p=Path(sys.argv[1]); v=json.loads((p/"package.json").read_text())["version"]; assert v=="4.60100.0", f"Expected IDF package 4.60100.0; got {v}"' "$IDF_PATH"
[[ -f "${ESP_ROM_ELF_DIR}esp32_rev0_rom.elf" ]] || { echo 'ROM ELF package is missing.' >&2; exit 2; }
for tool in cmake ninja xtensa-esp32-elf-gcc; do command -v "$tool" >/dev/null; done
mode=${1:-check}
case "$mode" in
  check)
    "$python_bin" "$IDF_PATH/tools/idf.py" --version
    xtensa-esp32-elf-gcc --version
    cmake --version
    ninja --version
    "$python_bin" -m pip check
    ;;
  default|ota|ota-fail)
    output="$test_root/resume-20261031/idf/$mode"
    work="$test_root/idf-current/$mode"
    args=(--work-root "$work" --idf-path "$IDF_PATH" --python "$python_bin"
          --parallel 2 --timeout 1800 --output "$output/build-linux.json")
    if [[ "$mode" == ota ]]; then
      args+=(--match ota --defaults-overlay "$handbook_root/validation/ota-build-only.defaults")
    elif [[ "$mode" == ota-fail ]]; then
      args+=(--match ota --defaults-overlay "$handbook_root/validation/windows-repair/ota-selftest-fail.defaults")
    fi
    "$python_bin" "$handbook_root/scripts/build_idf_native.py" "${args[@]}"
    size_args=(--build-root "$work/builds" --output "$output/sizes.json")
    if [[ "$mode" != default ]]; then size_args+=(--match ota); fi
    "$python_bin" "$handbook_root/scripts/check_idf_sizes.py" "${size_args[@]}"
    ;;
  *) echo 'Usage: bash scripts/linux_idf.sh check|default|ota|ota-fail' >&2; exit 2 ;;
esac
