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
from stdCharged import stdPi, stdK, stdE, stdMu
from stdPhotons import *
from skimExpertFunctions import *
gb2_setuprel = 'release-03-00-00'
set_log_level(LogLevel.INFO)

skimpath = Path()

skimCode = encodeSkimName('Systematics')
argvs = sys.argv
argc = len(argvs)


fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('MC9', fileList, path=skimpath)

stdE('loose', path=skimpath)
stdMu('loose', path=skimpath)
stdPi('all', path=skimpath)
stdK('all', path=skimpath)
stdE('all', path=skimpath)
stdMu('all', path=skimpath)

from skim.systematics import SystematicsList
SysList = SystematicsList(skimpath)
skimOutputUdst(skimCode, SysList, path=skimpath)
summaryOfLists(SysList, path=skimpath)

if 'Validation' in argvs:
    ntupleFile('Validation_Jpsimumu.root', path=skimpath)
    toolsdstar = ['EventMetaData', '^J/psi -> mu+ mu-']
    toolsdstar += ['InvMass', '^J/psi -> mu+ mu-']
    toolsdstar += ['Kinematics', '^J/psi -> ^mu+ ^mu-']
    toolsdstar += ['Track', '^J/psi -> mu+ mu-']
    toolsdstar += ['MCTruth', '^J/psi -> mu+ mu-']
    toolsdstar += ['CMSKinematics', '^J/psi -> mu+ mu-']
    ntupleTree('Jpsimumu', 'J/psi:mumutagprobe0', toolsdstar, path=skimpath)

    ntupleFile('Validation_Jpsiee.root', path=skimpath)
    toolsdstar = ['EventMetaData', '^J/psi -> e+ e-']
    toolsdstar += ['InvMass', '^J/psi -> e+ e-']
    toolsdstar += ['Kinematics', '^J/psi -> ^e+ ^e-']
    toolsdstar += ['Track', '^J/psi -> e+ e-']
    toolsdstar += ['MCTruth', '^J/psi -> e+ e-']
    toolsdstar += ['CMSKinematics', '^J/psi -> e+ e-']
    ntupleTree('Jpsiee', 'J/psi:eetagprobe0', toolsdstar, path=skimpath)

    ntupleFile('Validation_Dstar.root', path=skimpath)
    toolsdstar = ['EventMetaData', '^D*+ -> D0 pi+']
    toolsdstar += ['InvMass', '^D*+ -> ^D0 pi+']
    toolsdstar += ['Kinematics', '^D*+ -> [^D0 -> ^K- ^pi+] ^pi+']
    toolsdstar += ['Track', '^D*+ -> ^D0 pi+']
    toolsdstar += ['MCTruth', '^D*+ -> ^D0 pi+']
    toolsdstar += ['CMSKinematics', '^D*+ -> ^D0 pi+']
    ntupleTree('Dstar', 'D*+:syst0', toolsdstar, path=skimpath)


setSkimLogging(path=skimpath)
process(skimpath)

print(statistics)
