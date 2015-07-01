#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys

from basf2 import *
from modularAnalysis import inputMdstList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from modularAnalysis import fillParticleList
from modularAnalysis import fillConvertedPhotonsList
from modularAnalysis import loadGearbox
from modularAnalysis import vertexKFit
from b2biiConversion import convertBelleMdstToBelleIIMdst

if len(sys.argv) != 3:
    sys.exit('Must provide two input parameters: [input_Belle_MDST_file][output_BelleII_ROOT_file].\n'
             'A small example Belle MDST file can be downloaded from'
             'http://www-f9.ijs.si/~zupanc/evtgen_exp_07_BptoD0pip-D0toKpipi0-0.mdst')

inputBelleMDSTFile = sys.argv[1]
outputBelle2ROOTFile = sys.argv[2]

# Convert
convertBelleMdstToBelleIIMdst(inputBelleMDSTFile)

# Reconstruct
# first the gearbox needs to be loaded
loadGearbox()

fillParticleList('pi+:all', '')
fillParticleList('K-:all', '')

fillParticleList('K_S0:mdst', '')
# vertexKFit('K_S0:mdst', 0.0)
matchMCTruth('K_S0:mdst')

reconstructDecay('D0:Kpipi0 -> K-:all pi+:all pi0:mdst', '1.7 < M < 2.0')
reconstructDecay('B+:D0pi -> anti-D0:Kpipi0 pi+:all', '4.8 < M < 5.5')

matchMCTruth('B+:D0pi')

# create and fill flat Ntuple with MCTruth and kinematic information
toolsK0 = ['EventMetaData', '^K_S0']
toolsK0 += ['Kinematics', '^K_S0 -> ^pi+ ^pi-']
toolsK0 += ['InvMass', '^K_S0']
toolsK0 += ['Vertex', '^K_S0']
# toolsK0 += ['MCVertex', '^K_S0']
toolsK0 += ['PID', 'K_S0 -> ^pi+ ^pi-']
toolsK0 += ['Track', 'K_S0 -> ^pi+ ^pi-']
# toolsK0 += ['TrackHits', 'K_S0 -> ^pi+ ^pi-']
toolsK0 += ['MCTruth', '^K_S0 -> ^pi+ ^pi-']
toolsK0 += ['CustomFloats[dr:dz:isSignal:chiProb]', '^K_S0']

toolsB = ['EventMetaData', '^B+']
toolsB += ['InvMass', '^B+ -> ^anti-D0 pi+']
toolsB += ['InvMass[BeforeFit]', '^B+ -> [anti-D0 -> K- pi+ ^pi0] pi+']
toolsB += ['DeltaEMbc', '^B+']
toolsB += ['Cluster', 'B+ -> [anti-D0 -> K- pi+ [pi0 -> ^gamma ^gamma]] pi+']
toolsB += ['MCTruth', '^B+ -> ^anti-D0 pi+']
toolsB += ['CustomFloats[isSignal]', '^B+ -> ^anti-D0 pi+']
toolsB += ['CustomFloats[Kid_belle]', 'B+ -> [anti-D0 -> ^K- ^pi+ pi0] ^pi+']

toolsTrackPI = ['EventMetaData', 'pi+']
toolsTrackPI += ['Kinematics', '^pi+']
toolsTrackPI += ['Track', '^pi+']
toolsTrackPI += ['PID', '^pi+']
toolsTrackPI += ['MCTruth', '^pi+']
toolsTrackPI += ['MCKinematics', '^pi+']
toolsTrackPI += ['ErrMatrix', '^pi+']

ntupleFile(outputBelle2ROOTFile)
# ntupleTree('bp', 'B+:D0pi', toolsB)
ntupleTree('kshort', 'K_S0:mdst', toolsK0)
ntupleTree('pion', 'pi+:all', toolsTrackPI)

# progress
progress = register_module('Progress')
analysis_main.add_module(progress)

process(analysis_main)

# Print call statistics
print statistics
