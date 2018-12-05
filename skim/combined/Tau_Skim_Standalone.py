#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Combined Tau skim
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *
from skim.standardlists.lightmesons import *
from stdPi0s import *
from stdV0s import *
from skimExpertFunctions import *

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-02-00-01'

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

# TauGeneric skim
tauskim = Path()

inputMdstList('MC9', fileList, path=tauskim)
stdPi('all', path=tauskim)
stdPhotons('all', path=tauskim)

from skim.taupair import TauList
add_skim('TauGeneric', TauList(path=tauskim), path=tauskim)

setSkimLogging(path=tauskim)
process(tauskim)

print(statistics)

# TauLFV skim
taulfvskim = Path()

inputMdstList('MC9', fileList, path=taulfvskim)
stdPi('loose', path=taulfvskim)
stdK('loose', path=taulfvskim)
stdPr('loose', path=taulfvskim)
stdE('loose', path=taulfvskim)
stdMu('loose', path=taulfvskim)
stdPhotons('loose', path=taulfvskim)
stdPi0s('loose', path=taulfvskim)
loadStdSkimPi0(path=taulfvskim)
stdKshorts(path=taulfvskim)
loadStdLightMesons(path=taulfvskim)

from skim.taupair import TauLFVList
add_skim('TauLFV', TauLFVList(1, path=taulfvskim), path=taulfvskim)

setSkimLogging(path=taulfvskim)
process(taulfvskim)

print(statistics)
