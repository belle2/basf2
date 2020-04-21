#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Run all low multiplicity skims at once
#
#######################################################

import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert
from skim import lowMulti

lowmultiskimpath = b2.Path()

fileList = expert.get_test_file("MC13_mixedBGx1")
ma.inputMdstList('default', fileList, path=lowmultiskimpath)

expert.add_skim(
    'TwoTrackLeptonsForLuminosity',
    lowMulti.TwoTrackLeptonsForLuminosityList(
        path=lowmultiskimpath),
    path=lowmultiskimpath)
expert.add_skim('LowMassTwoTrack', lowMulti.LowMassTwoTrackList(path=lowmultiskimpath), path=lowmultiskimpath)

expert.setSkimLogging(lowmultiskimpath, ['ParticleCombiner'])
b2.process(lowmultiskimpath)

# print out the summary
print(b2.statistics)
