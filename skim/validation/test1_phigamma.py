#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Skim for ISR phi gamma events, with phi-->K+K- or KS(pi+pi-)KL
# G. Finocchiaro 2020
#
#######################################################

import basf2 as b2
import modularAnalysis as ma
import stdCharged as charged
from stdPhotons import stdPhotons

import skimExpertFunctions as expert
skimCode = expert.encodeSkimName('phigamma')

b2.set_log_level(b2.LogLevel.INFO)

phigamma_path = b2.Path()

fileList = ['./phigamma_neutral.dst.root']
print("fileList="+''.join(fileList))
ma.inputMdstList('default', fileList, path=phigamma_path)

charged.stdPi('all', path=phigamma_path)
charged.stdK('all',  path=phigamma_path)
stdPhotons('loose',  path=phigamma_path)

from skim.skim_phigamma import PhiGammaLooseList

phigamma_list = PhiGammaLooseList(phigamma_path)

outputUdstFile = "./phigamma_neutral_skimmed"
print("outputFile="+outputUdstFile)
expert.skimOutputUdst(skimCode, phigamma_list, path=phigamma_path, outputFile=outputUdstFile)
ma.summaryOfLists(phigamma_list, path=phigamma_path)

expert.setSkimLogging(path=phigamma_path)
b2.process(phigamma_path)

print(b2.statistics)
