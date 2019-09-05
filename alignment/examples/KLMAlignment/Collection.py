#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# KLM alignment: collection.

import sys
import basf2

# Create path.
main = basf2.create_path()

# Input.
main.add_module('RootInput', inputFileName=sys.argv[1])

# Histogram management.
main.add_module('HistoManager', histoFileName=sys.argv[2])

# Geometry and XML data.
main.add_module('Gearbox')
main.add_module('Geometry')

# Genfit extrapolation.
main.add_module('SetupGenfitExtrapolation',
                noiseBetheBloch=False,
                noiseCoulomb=False,
                noiseBrems=False)

# DAF fitter.
main.add_module('DAFRecoFitter', resortHits=True)

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
