#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# KLM event T0 with its validation: KLMEventT0Estimator computes the KLM event T0 and
# KLMEventT0ValidationModule fills the validation histograms from the estimator's results.
# Usage: basf2 KLMEventT0WithValidation.py -i <cdst file(s)>
# The global tags must provide the KLM time calibration payloads, including KLMEventT0HitResolution.

import basf2
import modularAnalysis as ma
from reconstruction import prepare_cdst_analysis
from KLMEventT0Validation import KLMEventT0ValidationModule

muon_list = 'mu+:forT0'

main = basf2.create_path()
main.add_module('RootInput')
main.add_module('HistoManager', histoFileName='KLMEventT0Estimator.root')
main.add_module('Gearbox')
main.add_module('Geometry')
prepare_cdst_analysis(path=main)
ma.fillParticleList(muon_list, 'p > 1.5 and abs(d0) < 2 and abs(z0) < 4', path=main)
main.add_module('KLMEventT0Estimator', MuonListName=muon_list)
main.add_module(KLMEventT0ValidationModule(muon_list_name=muon_list, output_file='KLMEventT0Validation.root'))

basf2.process(main, calculateStatistics=True)
print(basf2.statistics)
