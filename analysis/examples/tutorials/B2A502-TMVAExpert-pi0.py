#!/usr/bin/env python
# -*- coding: utf-8 -*-

#######################################################
#
# This tutorial demonstrates how to apply TMVA expertise
# (that was previously trained) on pi0 candidates using the
# following input variables:
#  - CMS momentum of the pi0
#  - polar angle of the pi0
#  - estimated uncertainty of the gammagamma invariant mass
#  - angle between one of the photon's momentum in pi0 frame and pi0 momentum in lab frame
#  - energy of the first daughter photon
#  - energy of the second daughter photon
#  - E9/E25 of the first daughter photon
#  - E9/E25 of the second daughter photon
#
# Note: This example uses the generic MC sample created in
# MC campaign 3.5, therefore it can be ran only on KEKCC computers.
#
# Contributors: A. Zupanc (June 2014)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdst
from modularAnalysis import loadReconstructedParticles
from modularAnalysis import reconDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from stdFSParticles import goodPhoton

from modularAnalysis import *

filelistMIX = \
    ['/group/belle2/MC/generic/mixed/mcprod1405/BGx1/mc35_mixed_BGx1_s01/mixed_e0001r0010_s01_BGx1.mdst.root'
     ]
filelistCHG = \
    ['/group/belle2/MC/generic/charged/mcprod1405/BGx1/mc35_charged_BGx1_s01/charged_e0001r0010_s01_BGx1.mdst.root'
     ]
filelistCC = \
    ['/group/belle2/MC/generic/ccbar/mcprod1405/BGx1/mc35_ccbar_BGx1_s01/ccbar_e0001r0010_s01_BGx1.mdst.root'
     ]
filelistSS = \
    ['/group/belle2/MC/generic/ssbar/mcprod1405/BGx1/mc35_ssbar_BGx1_s01/ssbar_e0001r0010_s01_BGx1.mdst.root'
     ]
filelistDD = \
    ['/group/belle2/MC/generic/ddbar/mcprod1405/BGx1/mc35_ddbar_BGx1_s01/ddbar_e0001r0010_s01_BGx1.mdst.root'
     ]
filelistUU = \
    ['/group/belle2/MC/generic/uubar/mcprod1405/BGx1/mc35_uubar_BGx1_s01/uubar_e0001r0010_s01_BGx1.mdst.root'
     ]

inputMdstList(filelistMIX + filelistCHG + filelistCC + filelistSS + filelistDD
              + filelistUU)

# do the analysis
loadReconstructedParticles()

# create final state particle lists
# use standard-good photon candidates
# (good photons pass good ECL selection criteria)
# creates "gamma:good" list
goodPhoton()

# construct pi0 candidates
reconDecay('pi0 -> gamma:good gamma:good', '0.1 < M < 0.18')
# perform mass fit
massKFit('pi0', 0.0, '')
# perform MC matching
matchMCTruth('pi0')

# at the moment the python wrapper function for the TMVATeacher module
# does not exist, therefore we have to register the module by ourselves
# and set all module parameters
expert = register_module('TMVAExpert')
expert.param('prefix', 'PI0-B2A501-TMVA')
# the expertise is located in the current directory
expert.param('workingDirectory', './')
expert.param('method', 'BDTG')
expert.param('listNames', ['pi0'])
expert.param('signalProbabilityName', 'BDT')
analysis_main.add_module(expert)

toolsPI0 = ['MCTruth', '^pi0 -> gamma gamma']
toolsPI0 += ['Kinematics', '^pi0 -> ^gamma ^gamma']
toolsPI0 += ['MassBeforeFit', '^pi0']
toolsPI0 += ['MCKinematics', '^pi0 -> gamma gamma']
toolsPI0 += ['EventMetaData', '^pi0']
toolsPI0 += ['Cluster', 'pi0 -> ^gamma ^gamma']
toolsPI0 += ['CustomFloats[getExtraInfo(BDT)]', '^pi0']

# write flat ntuples

ntupleFile('B2A502-TMVAExpert-pi0.root')
ntupleTree('pi0', 'pi0', toolsPI0)

process(analysis_main)
print statistics
