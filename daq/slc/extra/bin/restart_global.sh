#!/bin/sh

cd ~/belle2/release/
. daq/slc/extra/bin/nsm2_global.sh
if [ $2 = "kill" ]; then killall $1;fi
$1 $2 $3


