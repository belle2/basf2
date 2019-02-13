#!/usr/bin/env bash

# This script will check how often the output files generated
# by the validation scripts of the validation package
# are actually used by the validation efforts of other
# packages.

set -e

wd=$(pwd)

function cleanup(){
    cd "${wd}"
}

# Go to previous dir at exit
trap cleanup EXIT

thisdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "${thisdir}"

# Find all root files that are produced by the validation scripts
# of the validation package by using grep.
# Warning: Only scripts declared in the header using <output>...</outpout>
# will be found.

extensions="*.C *.py"
regex='<output>.*</output>'
output_files=$(\
    grep --only-matching --no-filename --regexp  "${regex}" ${extensions} | \
    sed "s/<output>//g" | \
    sed "s/<\/output>//g" | \
    sed "s/,/ /g" \
)

cd "${thisdir}/../../"

# Now we loop over all of these output files and look for them in the
# software repository.

for file in ${output_files}; do
    echo "==========================================="
    echo ${file}
    echo "==========================================="
    grep --color -r ${file}
done
