#!/bin/sh

cd ~/belle2/release/
source ../tools/setup_belle2
setuprel
source daq/slc/extra/bin/nsm2_global.sh
$1 $2 $3
sleep 1000

