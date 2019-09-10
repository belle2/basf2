#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# KLM alignment: reconstruction and collection.

import sys
import basf2
from reconstruction import add_reconstruction

basf2.conditions.append_testing_payloads('localdb/database.txt')

# Create path.
main = basf2.create_path()

# Input.
main.add_module('RootInput', inputFileName=sys.argv[1])

# Histogram management.
main.add_module('HistoManager', histoFileName=sys.argv[2])

# Geometry and XML data.
main.add_module('Gearbox')
main.add_module('Geometry')

# Reconstruction.
add_reconstruction(main, pruneTracks=False, add_muid_hits=True)
# Disable the time window in muid module by setting it to 1 second.
# This is necessary because the  alignment needs to be performed before
# the time calibration; if the time window is not disabled, then all
# scintillator hits are rejected.
basf2.set_module_parameters(main, 'Muid', MaxDt=1e9)

# DAF fitter.
main.add_module('DAFRecoFitter', resortHits=True)

# Genfit extrapolation.
main.add_module('SetupGenfitExtrapolation',
                noiseBetheBloch=False,
                noiseCoulomb=False,
                noiseBrems=False)

# Millepede collector.
main.add_module('MillepedeCollector',
                components=['BKLMAlignment', 'EKLMAlignment',
                            'EKLMSegmentAlignment'],
                useGblTree=True,
                minPValue=1e-5)

# Progress.
main.add_module('Progress')

# Processing.
basf2.process(main)

# Print call statistics.
print(basf2.statistics)
