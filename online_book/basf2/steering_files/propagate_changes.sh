#!/usr/bin/env bash

set -e

changed_file=$1
target_files="${@:2}"
tmp_patch_name="patch.tmp"

git diff HEAD^ -- ${changed_file} > ${tmp_patch_name}

for file in ${target_files}; do
    echo "Applying patch to ${file}"
    patch ${file} ${tmp_patch_name}
done

rm ${tmp_patch_name}
