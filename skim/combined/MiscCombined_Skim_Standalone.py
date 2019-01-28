#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Run TCPV, Tau Generic and Tau LFV skims at once
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdPr, stdE, stdMu
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skim.standardlists.lightmesons import *
from skim.standardlists.dileptons import *
from skimExpertFunctions import setSkimLogging, add_skim, encodeSkimName
gb2_setuprel = 'release-03-00-00'
skimpath = Path()
fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList, path=skimpath)


stdPi('loose', path=skimpath)
stdK('loose', path=skimpath)
stdPr('loose', path=skimpath)
stdE('loose', path=skimpath)
stdMu('loose', path=skimpath)
stdPi('all', path=skimpath)
stdPi0s('loose', path=skimpath)
stdPhotons('loose', path=skimpath)
stdKshorts(path=skimpath)
loadStdLightMesons(path=skimpath)
loadStdSkimPi0(path=skimpath)
loadStdSkimPhoton(path=skimpath)

loadStdD0(path=skimpath)
loadStdDplus(path=skimpath)
loadStdDstar0(path=skimpath)
loadStdDstarPlus(path=skimpath)
loadStdDiLeptons(True, path=skimpath)

cutAndCopyList('gamma:E15', 'gamma:loose', '1.4<E<4', path=skimpath)


# BtoPi0Pi0 Skim
# from skim.btocharmless import BtoPi0Pi0List
# add_skim('BtoPi0Pi0', BtoPi0Pi0List(path=skimpath))
# Tau Skim
from skim.taupair import TauLFVList
add_skim('TauLFV', TauLFVList(path=skimpath), path=skimpath)

# TCPV Skim
from skim.tcpv import TCPVList
add_skim('TCPV', TCPVList(path=skimpath), path=skimpath)


# Tau Generic
from skim.taupair import TauList
add_skim('TauGeneric', TauList(path=skimpath), path=skimpath)


setSkimLogging(path=skimpath)
process(path=skimpath)

# print out the summary
print(statistics)
