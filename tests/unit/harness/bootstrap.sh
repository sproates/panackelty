#!/bin/sh
. tests/unit/harness/common.sh
case_name=corrupt-seed-before-bootstrap
# BUILD_DIR alone does not relocate the root panack-vm target. Run the
# destructive bootstrap probe in its own checkout so parallel CLI tests keep
# their executable; verify the shared inode as well as its contents below.
shared_vm_inode=$(ls -di "$root/panack-vm" | awk '{ print $1 }')
cp "$root/panack-vm" "$work/shared-vm-bytes"
checkout=$work/checkout
mkdir "$checkout"
for name in Makefile VERSION src tests; do
    ln -s "$root/$name" "$checkout/$name"
done
cp bootstrap/compiler-v9.bc "$work/compiler.bc"
printf '\377' | dd of="$work/compiler.bc" bs=1 count=1 conv=notrunc 2>/dev/null
cd "$checkout"
capture failure 60 make bootstrap "BUILD_DIR=$work/build" "SEED_COMPILER=$work/compiler.bc"
contains "$work/stderr" 'not a Panackelty bytecode file'
test ! -e "$work/build/bootstrap/stage2/compiler.bc" || fail 'corrupt seed reached stage two'
pass
case_name=bootstrap-preserves-shared-vm
test "$(ls -di "$root/panack-vm" | awk '{ print $1 }')" = "$shared_vm_inode" || fail 'bootstrap replaced shared VM'
equal_files "$root/panack-vm" "$work/shared-vm-bytes"
pass
