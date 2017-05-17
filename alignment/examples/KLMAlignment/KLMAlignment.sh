#!/bin/bash

# Example of KLM alignment.

# Generation of 100000 e+ e- -> mu+ mu- events.
basf2 -n 100000 ${BELLE2_LOCAL_DIR}/eklm/examples/TimeCalibration/Generation.py eetomumu_gen.root

# Simulation and reconstruction.
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/KLMAlignment/SimulationReconstruction.py eetomumu_gen.root eetomumu_rec.root

# Millipede collection.
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/KLMAlignment/Collection.py eetomumu_rec.root eetomumu_col.root

# Alignment.
basf2 ${BELLE2_LOCAL_DIR}/alignment/examples/KLMAlignment/Alignment.py eetomumu_col.root

# Comparison of the alignment results and initial displacement.
root -b -q ${BELLE2_LOCAL_DIR}'/alignment/examples/KLMAlignment/CompareAlignment.C("EKLMDisplacement.root","alignment.root","comparison.root")'
