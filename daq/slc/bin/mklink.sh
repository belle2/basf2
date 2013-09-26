#!/bin/bash

rm -f *d *.h nsmd2 dqmserver
ln -s ../nsm2/daemon/nsmd2
ln -s ../runcontrold/bin/runcontrold
ln -s ../cprcontrold/bin/cprcontrold
ln -s ../ttdcontrold/bin/ttdcontrold
ln -s ../rocontrold/bin/rocontrold
ln -s ../templated/bin/tempd
ln -s ../include/nsm/*.h .
ln -s ../tools/dqmserver/bin/dqmserver
