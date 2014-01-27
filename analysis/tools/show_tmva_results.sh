#!/bin/bash
# show_tma_results.sh TMVA.root
# Shows Control Plots for a training stored in root file created by TMVATeacher
root -l "$BELLE2_EXTERNALS_DIR/src/root/tmva/test/TMVAGui.C(\"$1\")"
