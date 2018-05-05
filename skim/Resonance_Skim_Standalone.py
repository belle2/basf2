#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#######################################################
#
# Resonance rediscovery skim
# Y. Kato, Mar/2018
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPi0s import *
from stdPhotons import *
from skimExpertFunctions import *


set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-02-00-00'

import sys
import os
import glob

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]
inputMdstList('default', fileList)

argvs = sys.argv
argc = len(argvs)

loadStdCharged()
stdPi0s('looseFit')


scriptName = sys.argv[0]
skimListName = scriptName[:-19]
skimCode = encodeSkimName(skimListName)

from Resonance_List import *
ResonanceList = ResonanceList()
if 'Validation' in argvs and argc > 2:
    skimOutputUdst('%s_%s' % (skimCode, argvs[argvs.index('Validation') + 1]), ResonanceList)
else:
    skimOutputUdst(skimCode, ResonanceList)

summaryOfLists(ResonanceList)

if 'Validation' in argvs:
    if argc > 2:
        ntupleFile('Validation_%s_%s.root' % (skimCode, (argvs[argvs.index('Validation') + 1])))
    else:
        ntupleFile('Validation_%s.root' % (skimCode))

    toolsds = ['EventMetaData', '^D_s+']
    toolsds += ['InvMass', '^D_s+ -> ^phi pi+']
    toolsds += ['Kinematics', '^D_s+']
    toolsds += ['Track', '^D_s+']
    toolsds += ['MCTruth', '^D_s+']
    toolsds += ['CMSKinematics', '^D_s+']
    ntupleTree('Ds', 'D_s+:Resonance0', toolsds)

    toolsdstar = ['EventMetaData', '^D*+ -> ^D+ pi0']
    toolsdstar += ['InvMass', '^D*+ -> ^D+ ^pi0']
    toolsdstar += ['Kinematics', '^D*+ -> ^D+ pi0']
    toolsdstar += ['Track', '^D*+ -> ^D+ pi0']
    toolsdstar += ['MCTruth', '^D*+ -> ^D+ pi0']
    toolsdstar += ['CMSKinematics', '^D*+ -> ^D+ pi0']
    toolsdstar += ['CustomFloats[massDifference(0)]', '^D*+ -> D+ pi0']
    ntupleTree('Dstar', 'D*+:resonance0', toolsdstar)

    toolssigmacplus = ['EventMetaData', '^Sigma_c++ -> ^Lambda_c+ pi+']
    toolssigmacplus += ['InvMass', '^Sigma_c++ -> ^Lambda_c+ pi+']
    toolssigmacplus += ['Kinematics', ' ^Sigma_c++ -> ^Lambda_c+ pi+']
    toolssigmacplus += ['Track', ' ^Sigma_c++ -> ^Lambda_c+ pi+']
    toolssigmacplus += ['MCTruth', ' ^Sigma_c++ -> ^Lambda_c+ pi+']
    toolssigmacplus += ['CMSKinematics', '^Sigma_c++ -> ^Lambda_c+ pi+']
    toolssigmacplus += ['PID', 'Sigma_c++ -> [Lambda_c+ -> ^p+ ^K- ^pi+] pi+']
    toolssigmacplus += ['CustomFloats[massDifference(0)]', '^Sigma_c++ -> Lambda_c+ pi+']
    ntupleTree('Sigma_cplus', 'Sigma_c++:resonance0', toolssigmacplus)

    toolssigmaczero = ['EventMetaData', '^Sigma_c0 -> ^Lambda_c+ pi-']
    toolssigmaczero += ['InvMass', '^Sigma_c0 -> ^Lambda_c+ pi-']
    toolssigmaczero += ['Kinematics', ' ^Sigma_c0 -> ^Lambda_c+ pi-']
    toolssigmaczero += ['Track', ' ^Sigma_c0 -> ^Lambda_c+ pi-']
    toolssigmaczero += ['MCTruth', ' ^Sigma_c0 -> ^Lambda_c+ pi-']
    toolssigmaczero += ['CMSKinematics', '^Sigma_c0 -> ^Lambda_c+ pi-']
    toolssigmaczero += ['PID', 'Sigma_c0 -> [Lambda_c+ -> ^p+ ^K- ^pi+] pi-']
    toolssigmaczero += ['CustomFloats[massDifference(0)]', '^Sigma_c0 -> Lambda_c+ pi-']
    ntupleTree('Sigma_czero', 'Sigma_c0:resonance0', toolssigmaczero)

    toolsb = ['EventMetaData', '^B+']
    toolsb += ['InvMass', '^B+ -> ^anti-D0 pi+']
    toolsb += ['Kinematics', '^B+ -> ^anti-D0 ^pi+']
    toolsb += ['DeltaEMbc', '^B+']
    toolsb += ['MCTruth', '^B+ -> ^anti-D0 pi+']
    ntupleTree('Bplus', 'B+:resonance0', toolsb)

    toolsb = ['EventMetaData', '^B0']
    toolsb += ['InvMass', '^B0 -> ^D- pi+']
    toolsb += ['Kinematics', '^B0 -> ^D- ^pi+']
    toolsb += ['DeltaEMbc', '^B0']
    toolsb += ['MCTruth', '^B0 -> ^D- pi+']
    ntupleTree('B0', 'B0:resonance0', toolsb)

    toolsv = ['EventMetaData', '^vpho']
    toolsv += ['RecoStats', '^vpho']
    toolsv += ['InvMass', '^vpho  ']
    toolsv += ['Kinematics', '^vpho -> ^mu+ ^mu-']
    toolsv += ['MCTruth', '^vpho -> ^mu+ ^mu-']
    ntupleTree('vpho', 'vpho:resonance0', toolsv)

for module in analysis_main.modules():
    if module.type() == 'ParticleLoader':
        module.set_log_level(LogLevel.ERROR)
    elif module.type() == 'ParticleVertexFitter':
        module.set_log_level(LogLevel.ERROR)

process(analysis_main)

print(statistics)
