#!/bin/sh

cd ~/belle2/release/
. ../tools/setup_belle2
setuprel
cd
restart_local.sh $1 $2 $3

