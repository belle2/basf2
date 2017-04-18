#!/bin/bash

# Example of KLM alignment.

# Displacements.
mkdir displacement
cd displacement

# Generation of zero displacements.
mkdir zero
cd zero
basf2 ${BELLE2_LOCAL_DIR}/eklm/examples/Alignment/ZeroDisplacement.py
cd ..

# Generation of random displacements.
mkdir random
cd random
basf2 ${BELLE2_LOCAL_DIR}/eklm/examples/Alignment/RandomDisplacement.py
cd ../..

# Generation of 100000 e+ e- -> mu+ mu- events.
basf2 -n 100000 ${BELLE2_LOCAL_DIR}/eklm/examples/TimeCalibration/Generation.py eetomumu_gen.root

# Simulation and reconstruction.
cp -R displacement/random/localdb .
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/KLMAlignment/SimulationReconstruction.py eetomumu_gen.root eetomumu_rec.root

# Millipede collection.
rm -rf localdb
cp -R displacement/zero/localdb .
# If an iteration is necessary, restart from here.
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/KLMAlignment/Collection.py eetomumu_rec.root eetomumu_col.root

# Alignment.
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/KLMAlignment/Alignment.py eetomumu_col.root

# Comparison of the alignment results and initial displacement.
root -b -q ${BELLE2_LOCAL_DIR}'/alignment/examples/KLMAlignment/CompareAlignment.C("EKLMDisplacement.root","alignment.root","comparison.root")'

