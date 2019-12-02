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

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdPi, stdPr, stdK, stdE, stdMu
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skim.standardlists.lightmesons import *
from skim.standardlists.dileptons import *
import skimExpertFunctions as expert
b2.set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'

fileList = expert.get_test_file("mixedBGx1", "MC12")

syspath = b2.Path()
ma.inputMdstList('default', fileList, path=syspath)
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
expert.add_skim('Systematics', SystematicsList(path=syspath), path=syspath)

# Systematics Lambda Skim
from skim.systematics import *
expert.add_skim('SystematicsLambda', SystematicsLambdaList(path=syspath), path=syspath)

# Systematics Tracking
from skim.systematics import *
expert.add_skim('SystematicsTracking', SystematicsTrackingList(path=syspath), path=syspath)

# Resonan ce
from skim.systematics import *
expert.add_skim('Resonance', ResonanceList(path=syspath), path=syspath)

# Systematics Rad mu mu
from skim.systematics import *
expert.add_skim('SystematicsRadMuMu', SystematicsRadMuMuList(path=syspath), path=syspath)

# Systematics Rad mu mu
from skim.systematics import *
expert.add_skim('SystematicsRadEE', SystematicsRadEEList(path=syspath), path=syspath)

b2.process(path=syspath)

expert.setSkimLogging(path=syspath)
# print out the summary
print(statistics)
