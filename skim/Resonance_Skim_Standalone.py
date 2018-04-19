#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# HIGH RETENTION RATE NOT PRODUCED
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

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-01-00-00'

import sys
import os
import glob

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


argvs = sys.argv
argc = len(argvs)


inputMdstList('default', fileList)
loadStdCharged()
stdPi0s('looseFit')


from Resonance_List import *
ResonanceList = ResonanceList()
skimOutputUdst('Resonance', ResonanceList)
summaryOfLists(ResonanceList)

if 'Validation' in argvs:
    ntupleFile('Validation_Resonance.root')
    toolsds = ['EventMetaData', '^D_s+']
    toolsds += ['InvMass', '^D_s+ -> ^phi pi+']
    toolsds += ['Kinematics', '^D_s+']
    toolsds += ['Track', '^D_s+']
    toolsds += ['MCTruth', '^D_s+']
    toolsds += ['CMSKinematics', '^D_s+']

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

    ntupleTree('Ds', 'D_s+:Resonance0', toolsds)

process(analysis_main)

print(statistics)
