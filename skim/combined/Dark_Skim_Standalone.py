#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Run all dark skims at once
#
#######################################################

import basf2 as b2
import modularAnalysis as ma
import stdCharged as charged
import stdPhotons as photons
import skimExpertFunctions as expert

gb2_setuprel = 'release-04-00-00'

darkskimpath = b2.Path()

fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=darkskimpath)

charged.stdMu('all', path=darkskimpath)
charged.stdE('all', path=darkskimpath)
photons.stdPhotons('all', path=darkskimpath)

from skim import dark
expert.add_skim('ALP3Gamma', dark.ALP3GammaList(path=darkskimpath), path=darkskimpath)
expert.add_skim('LFVZpVisible', dark.LFVZpVisibleList(path=darkskimpath), path=darkskimpath)
expert.add_skim('SinglePhotonDark', dark.SinglePhotonDarkList(path=darkskimpath), path=darkskimpath)
expert.add_skim('DimuonPlusMissingEnergy', dark.DimuonPlusMissingEnergyList(path=darkskimpath), path=darkskimpath)
expert.add_skim('ElectronMuonPlusMissingEnergy', dark.ElectronMuonPlusMissingEnergyList(path=darkskimpath), path=darkskimpath)
expert.add_skim('DielectronPlusMissingEnergy', dark.DielectronPlusMissingEnergyList(path=darkskimpath), path=darkskimpath)

expert.setSkimLogging(path=darkskimpath)
b2.process(darkskimpath)

# print out the summary
print(b2.statistics)
