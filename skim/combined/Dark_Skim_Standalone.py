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


darkskimpath = b2.Path()

fileList = expert.get_test_file("MC13_mixedBGx1")
ma.inputMdstList('default', fileList, path=darkskimpath)

charged.stdMu('all', path=darkskimpath)
charged.stdE('all', path=darkskimpath)
photons.stdPhotons('all', path=darkskimpath)

from skim import dark
expert.add_skim('ALP3Gamma', dark.ALP3GammaList(path=darkskimpath), path=darkskimpath)
expert.add_skim('LFVZpVisible', dark.LFVZpVisibleList(path=darkskimpath), path=darkskimpath)
expert.add_skim('SinglePhotonDark', dark.SinglePhotonDarkList(path=darkskimpath), path=darkskimpath)
expert.add_skim('GammaGammaControlKLMDark', dark.GammaGammaControlKLMDarkList(path=darkskimpath), path=darkskimpath)
expert.add_skim('DimuonPlusMissingEnergy', dark.DimuonPlusMissingEnergyList(path=darkskimpath), path=darkskimpath)
expert.add_skim('ElectronMuonPlusMissingEnergy', dark.ElectronMuonPlusMissingEnergyList(path=darkskimpath), path=darkskimpath)
expert.add_skim('TwoTrackLeptonsForLuminosity', dark.TwoTrackLeptonsForLuminosityList(path=darkskimpath), path=darkskimpath)
expert.add_skim('LowMassTwoTrack', dark.LowMassTwoTrackList(path=darkskimpath), path=darkskimpath)

expert.setSkimLogging(darkskimpath, ['ParticleCombiner'])
b2.process(darkskimpath)

# print out the summary
print(b2.statistics)
