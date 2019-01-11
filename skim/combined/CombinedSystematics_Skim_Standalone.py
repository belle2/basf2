#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Combined all systematic skims at once
#  * Final state particles
#  * Charged tracks
#  * Lambda
#  * Radiative Muon pair
#  * Resonance discovery
#
# R. Cheaib , May 28, 2018
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdPr, stdK, stdE, stdMu
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skim.standardlists.lightmesons import *
from skim.standardlists.dileptons import *
set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-03-00-00'

from skimExpertFunctions import setSkimLogging, encodeSkimName, add_skim
fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]
syspath = Path()
inputMdstList('MC9', fileList, path=syspath)
stdPi('loose', path=syspath)
stdK('loose', path=syspath)
stdE('loose', path=syspath)
stdMu('loose', path=syspath)
stdPi('all', path=syspath)
stdK('all', path=syspath)
stdE('all', path=syspath)
stdMu('all', path=syspath)
stdPr('loose', path=syspath)
stdPi0s('looseFit', path=syspath)
stdPhotons('loose', path=syspath)


# Systematics skim
from skim.systematics import *
add_skim('Systematics', SystematicsList(path=syspath), path=syspath)

# Systematics Lambda Skim
from skim.systematics import *
add_skim('SystematicsLambda', SystematicsLambdaList(path=syspath), path=syspath)

# Systematics Tracking
from skim.systematics import *
add_skim('SystematicsTracking', SystematicsTrackingList(path=syspath), path=syspath)

# Resonan ce
from skim.systematics import *
add_skim('Resonance', ResonanceList(path=syspath), path=syspath)

# Systematics Rad mu mu
from skim.systematics import *
add_skim('SystematicsRadMuMu', SystematicsRadMuMuList(path=syspath), path=syspath)

# Systematics Rad mu mu
from skim.systematics import *
add_skim('SystematicsRadEE', SystematicsRadEEList(path=syspath), path=syspath)

process(path=syspath)

setSkimLogging(path=syspath)
# print out the summary
print(statistics)
