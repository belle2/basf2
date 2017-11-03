#!/bin/bash

# Example of KLM alignment.

# Generation of random displacements.
mkdir displacement
cd displacement
basf2 ${BELLE2_LOCAL_DIR}/eklm/examples/DisplacementGeneration/RandomDisplacement.py
cd ..

# Generation of 100000 e+ e- -> mu+ mu- events.
basf2 -n 100000 ${BELLE2_LOCAL_DIR}/eklm/examples/TimeCalibration/Generation.py eetomumu_gen.root

# Simulation.
cp -R displacement/localdb .
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/EKLMAlignment/Simulation.py eetomumu_gen.root eetomumu_sim.root
rm -rf localdb

# Reconstruction.
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/EKLMAlignment/Reconstruction.py eetomumu_sim.root eetomumu_rec.root

# Millipede collection. If an iteration is necessary, restart from here.
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/EKLMAlignment/Collection.py eetomumu_rec.root eetomumu_col.root

# Alignment.
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/EKLMAlignment/Alignment.py eetomumu_col.root

# Comparison of the alignment results and initial displacement.
root -b -q ${BELLE2_LOCAL_DIR}'/alignment/examples/EKLMAlignment/CompareAlignment.C("displacement/EKLMDisplacement.root","alignment.root","comparison.root")'

# Graphical comparison of alignment results and initial displacement.
root -b -q ${BELLE2_LOCAL_DIR}'/alignment/examples/EKLMAlignment/ViewAlignment.C("comparison.root")'
