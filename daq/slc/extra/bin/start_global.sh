#!/bin/sh

cd ~/belle2/release/
. ../tools/setup_belle2
setuprel
restart_global.sh $1 $2 $3


