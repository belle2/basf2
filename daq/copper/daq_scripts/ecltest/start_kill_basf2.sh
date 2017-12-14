#!/bin/sh
ssh -n -n -XY -l${USER} $1 ~/bin/kill_basf2_all.sh
