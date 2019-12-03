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
from stdCharged import stdE, stdK, stdMu, stdPi, stdPr
from stdPhotons import stdPhotons
from stdPi0s import stdPi0s
import skimExpertFunctions as expert
from skim.systematics import ResonanceList, SystematicsLambdaList, SystematicsList, SystematicsRadEEList, \
    SystematicsRadMuMuList, SystematicsTrackingList
b2.set_log_level(b2.LogLevel.INFO)
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
expert.add_skim('Systematics', SystematicsList(path=syspath), path=syspath)

# Systematics Lambda Skim
expert.add_skim('SystematicsLambda', SystematicsLambdaList(path=syspath), path=syspath)

# Systematics Tracking
expert.add_skim('SystematicsTracking', SystematicsTrackingList(path=syspath), path=syspath)

# Resonan ce
expert.add_skim('Resonance', ResonanceList(path=syspath), path=syspath)

# Systematics Rad mu mu
expert.add_skim('SystematicsRadMuMu', SystematicsRadMuMuList(path=syspath), path=syspath)

# Systematics Rad mu mu
expert.add_skim('SystematicsRadEE', SystematicsRadEEList(path=syspath), path=syspath)

b2.process(path=syspath)

expert.setSkimLogging(path=syspath)
# print out the summary
print(b2.statistics)
