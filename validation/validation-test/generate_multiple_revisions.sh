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

function fake_timestamp(){
    sed -i "s/\"creation_date\".*/\"creation_date\": \"${2}\",/g"  "results/${1}/revision.json"
}

function run(){
    b2validation --test -t "${1}"
    fake_timestamp "${1}" "${2}"
}

run "release-1999-01-29" "1999-01-29 10:15"
run "prerelease-2003-05-10" "2003-05-10 11:13"
run "release-2003-05-10" "2003-05-10 05:55"
run "build-2010-10-10" "2010-10-10 10:15"
run "build-2013-10-10" "2013-10-10 05:03"
run "nightly-2013-11-10" "2013-10-10 19:30"
run "nightly-2013-11-11" "2013-10-11 16:30"
run "nightly-2013-11-12" "2013-10-12 19:36"

