#!/bin/bash

# Example of KLM alignment.

# Generation of random displacements.
mkdir displacement
cd displacement
basf2 ${BELLE2_LOCAL_DIR}/klm/examples/displacement/RandomDisplacement.py
cd ..

# Generation of 100000 e+ e- -> mu+ mu- events.
basf2 -n 100000 ${BELLE2_LOCAL_DIR}/klm/eklm/examples/TimeCalibration/Generation.py eetomumu_gen.root

# Simulation and reconstruction.
# If an iteration is necessary, restart from here.
cp -R displacement/localdb .
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/KLMAlignment/Simulation.py eetomumu_gen.root eetomumu_rec.root
rm -rf localdb

# Millipede collection.
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/KLMAlignment/Collection.py eetomumu_rec.root eetomumu_col.root

# Alignment.
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/KLMAlignment/Alignment.py eetomumu_col.root

# Comparison of the alignment results and initial displacement.
root -b -q ${BELLE2_LOCAL_DIR}'/alignment/examples/KLMAlignment/CompareAlignment.C("displacement/EKLMDisplacement.root","alignment.root","comparison.root")'

# Graphical comparison of alignment results and initial displacement.
root -b -q ${BELLE2_LOCAL_DIR}'/alignment/examples/KLMAlignment/ViewAlignment.C("comparison.root")'
