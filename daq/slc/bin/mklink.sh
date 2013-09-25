#!/bin/bash

rm -f *d *.h
ln -s ../nsm2/daemon/nsmd2
ln -s ../runcontrold/bin/runcontrold
ln -s ../cprcontrold/bin/cprcontrold
ln -s ../ttdcontrold/bin/ttdcontrold
ln -s ../rocontrold/bin/rocontrold
ln -s ../templated/bin/tempd
ln -s ../include/nsm/*.h .
