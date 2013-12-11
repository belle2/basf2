#!/bin/sh

cd ~/belle2/release/
source ../tools/setup_belle2
setuprel
source daq/slc/extra/bin/nsm2_local.sh
if [ $2 = "kill" ]; then killall $1;fi
$1 $2 $3

