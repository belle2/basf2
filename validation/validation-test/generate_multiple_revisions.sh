#!/usr/bin/env bash

if [[ -z ${BELLE2_LOCAL_DIR} ]]; then
    (>&2 echo "Error: Belle2 software is not set up!")
    exit 1
fi

oldDir=$(pwd)

function cleanup(){
    cd "${oldDir}"
}

trap cleanup EXIT

cd "${BELLE2_LOCAL_DIR}"

cmd="b2validation --test -t"

${cmd} release-1999-01-29
${cmd} prerelease-2003-05-10
${cmd} release-2003-05-10
${cmd} build-2010-10-10
${cmd} build-2013-10-10
${cmd} nightly-2011-10-10
