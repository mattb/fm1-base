#!/bin/sh
# SPDX-License-Identifier: MPL-2.0
set -eu
CC=${CC:-cc}
OUT=${1:-template/build/tests}
mkdir -p "$OUT"
FLAGS="-std=c99 -Wall -Wextra -Werror -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer"
$CC $FLAGS tests/sysex_test.c -o "$OUT/sysex_test"
"$OUT/sysex_test"
for size in 128 1024; do
    $CC $FLAGS -DSXQ=$size tests/sysex_host_test.c -o "$OUT/sysex_host_test"
    FM1B_SYSEX_LOG= "$OUT/sysex_host_test"
    FM1B_SYSEX_LOG="$OUT/sysex-$size.syx" "$OUT/sysex_host_test"
    FM1B_SYSEX_LOG="$OUT/missing-directory/capture.syx" "$OUT/sysex_host_test" fail
done
$CC $FLAGS -DPLAT_WEB tests/sysex_host_test.c -o "$OUT/sysex_web_test"
"$OUT/sysex_web_test"
