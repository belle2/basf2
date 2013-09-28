#!/bin/bash

rm -f *d *.h nsmd2 dqmserver
if [ -e nsmd2 ]; ln -s ../nsm2/daemon/nsmd2
if [ -e dqmservd ]; ln -s ../dqmserver/bin/dqmservd
ln -s ../runcontrold/bin/runcontrold
ln -s ../cprcontrold/bin/cprcontrold
ln -s ../ttdcontrold/bin/ttdcontrold
ln -s ../rocontrold/bin/rocontrold
ln -s ../templated/bin/tempd
ln -s ../include/nsm/*.h .
