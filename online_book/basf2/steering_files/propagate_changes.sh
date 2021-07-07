#!/usr/bin/env bash

# THIS IS NOT A BASF2 EXAMPLE

# Example use case:
# You change something in 019_first_stering_file.py
# For consistency, this change needs to be applied to
# 029, ..., 059_vertex_fitting.py as well.
# Then run
# ./propagate_changes 019_first_steering_file 029_roe.py 039_various_additions.py <etc>
# To apply these changes there as well.
# IMPORTANT NOTE: THIS IS A BIT FRAGILE, SO MAKE SURE TO CHECK
# THAT THIS SCRIPT WORKS AS INTENDED!

changed_file=$1
target_files="${@:2}"
tmp_patch_name="patch.tmp"

git diff HEAD^ -- ${changed_file} > ${tmp_patch_name}

for file in ${target_files}; do
    echo "Applying patch to ${file}"
    patch -f --fuzz=2 ${file} ${tmp_patch_name}
done

rm ${tmp_patch_name}
