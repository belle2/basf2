#!/bin/bash
# show_tma_results.sh TMVA.root
# Shows Control Plots for a training stored in root file created by TMVATeacher

#get full path
FILE=`readlink -f "$1"`

cd $BELLE2_EXTERNALS_DIR/src/root/tmva/test/
if [ $? -ne 0 ]
then
  echo "Directory src/root/tmva/test not found in externals. Please re-install from source."
  exit 1
fi
root -l "$BELLE2_EXTERNALS_DIR/src/root/tmva/test/TMVAGui.C(\"$FILE\")"
