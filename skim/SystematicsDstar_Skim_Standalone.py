#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# P. Urquijo, 6/Jan/2015
# Modified by Y. Kato Mar, 2018
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *

set_log_level(LogLevel.INFO)

gb_setuprel = 'release-01-00-00'
import sys
import os
import glob

argvs = sys.argv
argc = len(argvs)

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('default', fileList)


loadStdCharged()

from SystematicsDstar_List import *
SysList = SystematicsList()
skimOutputUdst('SystematicsDstar', SysList)

summaryOfLists(SysList)

if 'Validation' in argvs:
    ntupleFile('Validation_Dstar.root')
    toolsdstar = ['EventMetaData', '^D*+ -> D0 pi+']
    toolsdstar += ['InvMass', '^D*+ -> ^D0 pi+']
    toolsdstar += ['Kinematics', '^D*+ -> [^D0 -> ^K- ^pi+] ^pi+']
    toolsdstar += ['Track', '^D*+ -> ^D0 pi+']
    toolsdstar += ['MCTruth', '^D*+ -> ^D0 pi+']
    toolsdstar += ['CMSKinematics', '^D*+ -> ^D0 pi+']
    ntupleTree('Dstar', 'D*+:syst0', toolsdstar)


for module in analysis_main.modules():
    if module.type() == "ParticleLoader":
        module.set_log_level(LogLevel.ERROR)
process(analysis_main)

print(statistics)
