#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# P. Urquijo, 6/Jan/2015
# Modified by Y. Kato, Mar/2018
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *
gb2_setuprel = 'release-01-00-00'
set_log_level(LogLevel.INFO)


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

from SystematicsJpsimumu_List import *
SysList = SystematicsList()
skimOutputUdst('SystematicsJpsimumu', SysList)
summaryOfLists(SysList)

if 'Validation' in argvs:
    ntupleFile('Validation_Jpsimumu.root')
    toolsdstar = ['EventMetaData', '^J/psi -> mu+ mu-']
    toolsdstar += ['InvMass', '^J/psi -> mu+ mu-']
    toolsdstar += ['Kinematics', '^J/psi -> ^mu+ ^mu-']
    toolsdstar += ['Track', '^J/psi -> mu+ mu-']
    toolsdstar += ['MCTruth', '^J/psi -> mu+ mu-']
    toolsdstar += ['CMSKinematics', '^J/psi -> mu+ mu-']
    ntupleTree('Jpsimumu', 'J/psi:mumutagprobe0', toolsdstar)

for module in analysis_main.modules():
    if module.type() == "ParticleLoader":
        module.set_log_level(LogLevel.ERROR)
process(analysis_main)

print(statistics)
