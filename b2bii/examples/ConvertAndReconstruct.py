#!/usr/bin/env python3
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
from modularAnalysis import vertexRave
from modularAnalysis import printVariableValues
from b2biiConversion import convertBelleMdstToBelleIIMdst, setupB2BIIDatabase
from b2biiMonitors import addBeamParamsConversionMonitors
from b2biiMonitors import addTrackConversionMonitors
from b2biiMonitors import addNeutralsConversionMonitors

if len(sys.argv) != 4:
    sys.exit('Must provide two input parameters: [mc|data] [input_Belle_MDST_file][output_BelleII_ROOT_file].\n'
             'A small example Belle MDST file can be downloaded from '
             'http://www-f9.ijs.si/~zupanc/evtgen_exp_07_BptoD0pip-D0toKpipi0-0.mdst')

mc_or_data = sys.argv[1].lower()
isMC = {"mc": True, "data": False}.get(mc_or_data, None)
if isMC is None:
    sys.exit('First parameter must be "mc" or "data" to indicate whether we run on MC or real data')

setupB2BIIDatabase(isMC)

inputBelleMDSTFile = sys.argv[2]
outputBelle2ROOTFile = sys.argv[3]

os.environ['USE_GRAND_REPROCESS_DATA'] = '1'

print('BELLE2_EXTERNALS_DIR     = ' + str(os.getenv('BELLE2_EXTERNALS_DIR')))
print('BELLE2_EXTERNALS_SUBDIR  = ' + str(os.getenv('BELLE2_EXTERNALS_SUBDIR')))
print('BELLE2_EXTERNALS_OPTION  = ' + str(os.getenv('BELLE2_EXTERNALS_OPTION')))
print('BELLE2_EXTERNALS_VERSION = ' + str(os.getenv('BELLE2_EXTERNALS_VERSION')))
print('BELLE2_LOCAL_DIR         = ' + str(os.getenv('BELLE2_LOCAL_DIR')))
print('BELLE2_RELEASE           = ' + str(os.getenv('BELLE2_RELEASE')))
print('BELLE2_OPTION            = ' + str(os.getenv('BELLE2_OPTION')))
print('BELLE_POSTGRES_SERVER    = ' + str(os.getenv('BELLE_POSTGRES_SERVER')))
print('USE_GRAND_REPROCESS_DATA = ' + str(os.getenv('USE_GRAND_REPROCESS_DATA')))
print('PANTHER_TABLE_DIR        = ' + str(os.getenv('PANTHER_TABLE_DIR')))
print('PGUSER                   = ' + str(os.getenv('PGUSER')))

# Convert
convertBelleMdstToBelleIIMdst(inputBelleMDSTFile)

# Reconstruct
# first the gearbox needs to be loaded
loadGearbox()

# Create monitoring histograms
addBeamParamsConversionMonitors()
addTrackConversionMonitors()
addNeutralsConversionMonitors()

# Only charged final state particles need to be loaded
# All photon and pi0 candidates are already loaded
# to 'gamma:mdst' and 'pi0:mdst' particle lists
fillParticleList('pi+:all', '')
fillParticleList('K-:all', '')
fillParticleList('mu+:all', '')
fillParticleList('e+:all', '')

# in the case of pi0 candidates in 'pi0:mdst' the mc truth matching
# needs to be executed
matchMCTruth('pi0:mdst')
matchMCTruth('K_S0:mdst')
vertexKFit('K_S0:mdst', -1)

# The Belle PID variables are: atcPIDBelle(sigHyp, bkgHyp), muIDBelle, and eIDBelle
printVariableValues('pi+:all', ['mcPDG', 'p', 'atcPIDBelle(3,2)', 'muIDBelle', 'muIDBelleQuality', 'eIDBelle', 'nSVDHits'])

printVariableValues('gamma:mdst', ['mcPDG', 'E', 'clusterE9E25'])
printVariableValues('pi0:mdst', ['mcPDG', 'p', 'M', 'InvM'])

# V0's (loaded by ParticleLoader from the converted V0 Array)
# the difference between K_S0:all and K_S0:mdst is in the
# momentum/position of the daughter tracks. The K_S0:all
# daughters have momentum/position determined wrt. pivot=(0,0,0),
# while K_S0:mdst daughters have it wrt. pivot=Decay vertex.
# In addition K_S0:mdst has goodKs and nisKsFinder outputs attached
# as extra info.
# fillParticleList('K_S0:all', '')
# vertexRave('K_S0:all', 0.0)
# matchMCTruth('K_S0:all')

printVariableValues('K_S0:mdst', ['mcPDG', 'M', 'InvM', 'p', 'px', 'py', 'pz',
                                  'extraInfo(goodKs)', 'extraInfo(ksnbVLike)', 'extraInfo(ksnbNoLam)'])

# reconstructDecay('D0:Kpipi0 -> K-:all pi+:all pi0:mdst', '1.7 < M < 2.0')
# reconstructDecay('B+:D0pi -> anti-D0:Kpipi0 pi+:all', '4.8 < M < 5.5')

# matchMCTruth('B+:D0pi')

# create and fill flat Ntuple with MCTruth and kinematic information
toolsK0 = ['EventMetaData', '^K_S0']
toolsK0 += ['Kinematics', '^K_S0 -> ^pi+ ^pi-']
toolsK0 += ['MomentumUncertainty', '^K_S0 -> ^pi+ ^pi-']
toolsK0 += ['InvMass', '^K_S0']
toolsK0 += ['Vertex', '^K_S0']
toolsK0 += ['MCVertex', '^K_S0']
# toolsK0 += ['PID', 'K_S0 -> ^pi+ ^pi-']
# toolsK0 += ['Track', 'K_S0 -> ^pi+ ^pi-']
# toolsK0 += ['TrackHits', 'K_S0 -> ^pi+ ^pi-']
toolsK0 += ['MCTruth', '^K_S0 -> ^pi+ ^pi-']
toolsK0 += [
    'CustomFloats[dr:dz:isSignal:chiProb:extraInfo(goodKs):extraInfo(ksnbVLike):extraInfo(ksnbNoLam):extraInfo(ksnbStandard)]',
    '^K_S0']

toolsB = ['EventMetaData', '^B+']
toolsB += ['InvMass', '^B+ -> ^anti-D0 pi+']
toolsB += ['InvMass[BeforeFit]', '^B+ -> [anti-D0 -> K- pi+ ^pi0] pi+']
toolsB += ['DeltaEMbc', '^B+']
toolsB += ['Cluster', 'B+ -> [anti-D0 -> K- pi+ [pi0 -> ^gamma ^gamma]] pi+']
toolsB += ['MCTruth', '^B+ -> ^anti-D0 pi+']
toolsB += ['CustomFloats[isSignal]', '^B+ -> ^anti-D0 pi+']
toolsB += ['CustomFloats[kIDBelle]', 'B+ -> [anti-D0 -> ^K- ^pi+ pi0] ^pi+']

toolsTrackPI = ['EventMetaData', 'pi+']
toolsTrackPI += ['Kinematics', '^pi+']
toolsTrackPI += ['Track', '^pi+']
# toolsTrackPI += ['PID', '^pi+']
toolsTrackPI += ['MCTruth', '^pi+']
toolsTrackPI += ['MCKinematics', '^pi+']
toolsTrackPI += ['ErrMatrix', '^pi+']
toolsTrackPI += ['CustomFloats[eIDBelle:muIDBelleQuality:muIDBelle:atcPIDBelle(3,2):atcPIDBelle(4,2):atcPIDBelle(4,3)]', '^pi+']

ntupleFile(outputBelle2ROOTFile)
# ntupleTree('bp', 'B+:D0pi', toolsB)
ntupleTree('kshort', 'K_S0:mdst', toolsK0)
# ntupleTree('pion', 'pi+:all', toolsTrackPI)

# progress
progress = register_module('Progress')
analysis_main.add_module(progress)

process(analysis_main)

# Print call statistics
print(statistics)
