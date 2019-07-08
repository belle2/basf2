#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# EWP B->Xgamma inclusive skim
# T.R. Shillington June 2019
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdPhotons import *
from stdCharged import stdPi
from skimExpertFunctions import setSkimLogging, encodeSkimName, get_test_file

gb2_setuprel = 'release-03-02-02'
skimCode = encodeSkimName('BtoXgamma')

fileList = get_test_file("mixedBGx1", "MC12")
path = Path()
inputMdstList('default', fileList, path=path)

stdPhotons('loose', path=path)
stdPhotons('all', path=path)
stdPi('all', path=path)

# Create lists for buildEventShape (basically all tracks and clusters)
cutAndCopyList('pi+:R2', 'pi+:all', 'pt> 0.1', path=path)
cutAndCopyList('gamma:R2', 'gamma:all', 'E > 0.1', path=path)

# buildEventShape to access R2
buildEventShape(inputListNames=['pi+:R2', 'gamma:R2'],
                allMoments=False,
                foxWolfram=True,
                harmonicMoments=False,
                cleoCones=False,
                thrust=False,
                collisionAxis=False,
                jets=False,
                sphericity=False,
                checkForDuplicates=False,
                path=path)

applyEventCuts('foxWolframR2 < 0.5 and nTracks >= 3', path=path)

cutAndCopyList('gamma:ewp', 'gamma:loose', 'clusterE9E21 > 0.9 and 1.4 < useCMSFrame(E) < 3.4', path=path)

reconstructDecay('B0:ewp -> gamma:ewp', '', path=path)
reconstructDecay('B+:ewp -> gamma:ewp', '', path=path)

XgammaList = ['B0:ewp', 'B+:ewp']

skimOutputUdst(skimCode, XgammaList, path=path)
summaryOfLists(XgammaList, path=path)


setSkimLogging(path=path)
process(path=path)

# print out the summary
print(statistics)
