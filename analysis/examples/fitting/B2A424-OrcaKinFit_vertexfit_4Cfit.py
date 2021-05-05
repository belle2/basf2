#!/usr/bin/env python3

###################################################################
# This tutorial demonstrates how to perform vertexfit with Rave
# and four momentum constraint fit with the OrcaKinFit. In this
# example the following decay chain:
#
# Upsilon(4S) -> gamma    A
#                         |
#                         +-> u+ u-
#
# is reconstructed. The  vertexfit is performed on u+ u-, and four
# momentum constraint fit is performed on all final states, and the
# total four momentum is set at that of cms.
#
# Contributors: Yu Hu (July 2018)
# yu.hu@desy.de
#
####################################################################

#
# Import and mdst loading
#
import basf2 as b2
from modularAnalysis import inputMdst
from modularAnalysis import fillParticleList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from vertex import raveFit
from kinfit import fitKinematic4C
from modularAnalysis import variablesToNtuple
import pdg
import variables.collections as vc
import variables.utils as vu

# create path
my_path = b2.create_path()

# load input ROOT file
inputMdst(environmentType='default',
          filename=b2.find_file('darkphotonmumu_mdst.root', 'examples', False),
          path=my_path)

# Creates a list of good photon and muons
fillParticleList("gamma:sel", 'E > 0.1 and abs(formula(clusterTiming/clusterErrorTiming)) < 1.0', path=my_path)
fillParticleList("mu-:sel", 'electronID < 0.01 and chiProb > 0.001 and abs(dz) < 3 and dr < 0.1', path=my_path)
pdg.add_particle('A', 9000008, 999., 999., 0, 0)  # name, PDG, mass, width, charge, spin
reconstructDecay("A:sel -> mu-:sel mu+:sel", "", path=my_path)
reconstructDecay("A:selvertex -> mu-:sel mu+:sel", "", path=my_path)

# Perform four momentum constraint fit using Rave and update the Daughters
raveFit("A:selvertex", -1.0, constraint="iptube", daughtersUpdate=True, path=my_path)

pdg.add_particle('beam', 9000009, 999., 999., 0, 0)  # name, PDG, mass, width, charge, spin
reconstructDecay("beam:sel -> A:sel gamma:sel", "", path=my_path)
reconstructDecay("beam:selv -> A:selvertex gamma:sel", "", path=my_path)
reconstructDecay("beam:selv4c -> A:selvertex gamma:sel", "", path=my_path)

# Associates the MC truth
matchMCTruth('beam:sel', path=my_path)
matchMCTruth('beam:selv', path=my_path)
matchMCTruth('beam:selv4c', path=my_path)

# Perform four momentum constraint fit using OrcaKinFit and update the Daughters
fitKinematic4C("beam:selv4c", path=my_path)

# Select variables that we want to store to ntuple
muvars = vc.kinematics + vc.mc_truth + vc.mc_kinematics + vc.pid + vc.momentum_uncertainty
gammavars = vc.inv_mass + vc.kinematics + vc.mc_kinematics + vc.mc_truth + vc.momentum_uncertainty
avars = vc.inv_mass + vc.kinematics + vc.mc_kinematics + vc.mc_truth + vc.momentum_uncertainty
uvars = vc.inv_mass + vc.kinematics + vc.mc_kinematics + vc.mc_truth + \
    vu.create_aliases_for_selected(muvars, 'beam -> [A -> ^mu+ ^mu-] gamma') + \
    vu.create_aliases_for_selected(gammavars, 'beam -> A ^gamma') + \
    vu.create_aliases_for_selected(avars, 'beam -> ^A gamma')

uvarsv = uvars + ['chiProb']

uvars4c = uvars + vu.create_aliases(['OrcaKinFitProb',
                                     'OrcaKinFitProb',
                                     'OrcaKinFitChi2',
                                     'OrcaKinFitErrorCode'], 'extraInfo({variable})', "") + \
    vu.create_aliases(['VertexFitChi2',
                       'VertexFitProb'], 'daughter(1, extraInfo({variable}))', "A")

# Saving variables to ntuple
output_file = 'B2A424-OrcaKinFit_vertexfit_4Cfit.root'
variablesToNtuple('beam:selv4c', uvars4c,
                  filename=output_file, treename='beamselv4c', path=my_path)
variablesToNtuple('beam:selv', uvarsv,
                  filename=output_file, treename='beamselv', path=my_path)
variablesToNtuple('beam:sel', uvarsv,
                  filename=output_file, treename='beamsel', path=my_path)

#
# Process and print statistics
#

# Process the events
b2.process(my_path)
# print out the summary
print(b2.statistics)
