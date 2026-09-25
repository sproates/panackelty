#!/bin/sh
. tests/unit/harness/common.sh
case_name=corrupt-seed-before-bootstrap
cp bootstrap/compiler-v8.bc "$work/compiler.bc"
printf '\377' | dd of="$work/compiler.bc" bs=1 count=1 conv=notrunc 2>/dev/null
capture failure 60 make bootstrap "BUILD_DIR=$work/build" "SEED_COMPILER=$work/compiler.bc"
contains "$work/stderr" 'not a Panackelty bytecode file'
test ! -e "$work/build/bootstrap/stage2/compiler.bc" || fail 'corrupt seed reached stage two'
pass
