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
sp = b2.Path()
fileList = expert.get_test_file("MC12_mixedBGx1")
ma.inputMdstList('default', fileList, path=darkskimpath)

charged.stdMu('all', path=darkskimpath)
charged.stdE('all', path=darkskimpath)
photons.stdPhotons('all', path=darkskimpath)


cleaned = 'abs(dz) < 2.0 and abs(dr) < 0.5 and pt > 0.15'  # cm, cm, GeV/c
minimum = 'E > 0.1'  # GeV
angle = '0.296706 < theta < 2.61799'  # rad, (17 -- 150 deg)
ma.cutAndCopyList('gamma:100', 'gamma:all', minimum + ' and ' + angle, path=darkskimpath)

from skim import dark
expert.add_skim('ALP3Gamma', dark.ALP3GammaList(path=darkskimpath), path=darkskimpath)
expert.add_skim('LFVZpVisible', dark.LFVZpVisibleList(path=darkskimpath), path=darkskimpath)
expert.outputIfPassesEventCut(
    'nCleanedTracks(' + cleaned + ') < 1 and 0 < nParticlesInList(gamma:100) < 2',
    conditionalPath=sp,
    skimName='SinglePhotonDark',
    lists=dark.SinglePhotonDarkList(
        path=darkskimpath),
    mainSkimPath=darkskimpath)
expert.add_skim('DimuonPlusMissingEnergy', dark.DimuonPlusMissingEnergyList(path=darkskimpath), path=darkskimpath)
expert.add_skim('ElectronMuonPlusMissingEnergy', dark.ElectronMuonPlusMissingEnergyList(path=darkskimpath), path=darkskimpath)
expert.add_skim('DielectronPlusMissingEnergy', dark.DielectronPlusMissingEnergyList(path=darkskimpath), path=darkskimpath)
expert.setSkimLogging(path=darkskimpath)
b2.process(darkskimpath)

# print out the summary
print(b2.statistics)
