#!/usr/bin/env bash

# Trains FEI on a provided input sample

#set your input file(s) here
INPUTFILE="/local/scratch/cpulvermacher/mcprod1405/mixed_e0001r00*"

set -e


# Execute repeatedly until summary PDF is generated
while [ ! -f FEIsummary.pdf ]; do
  # After the first run, reuse particles in tmpdata.root
  if [ -f tmpdata.root ]
  then
    INPUTFILE="tmpdata.root"
  fi
  basf2 $BELLE2_LOCAL_DIR/analysis/examples/FEI/B_generic.py -p8 -i "$INPUTFILE" -o tmpdata.root -- --cache cache.pkl --summary # -nThreads 8
done
