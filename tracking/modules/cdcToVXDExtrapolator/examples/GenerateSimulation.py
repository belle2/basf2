#!/usr/bin/env python
# -*- coding: utf-8 -*-

"Generate some example events with Kshorts for testing the CDCToVXDExtrapolator"

from basf2 import process, register_module, statistics
from modularAnalysis import generateY4S, analysis_main
from simulation import add_simulation
from reconstruction import add_mdst_output
from glob import glob

nEv = 100
generateY4S(nEv, 'kspipi.dec')

bkgdir = '/sw/belle2/bkg/'
bkgFiles = glob(bkgdir + '*root')

components = ['MagneticField', 'BeamPipe', 'PXD', 'SVD', 'CDC']
# components = None
gear = register_module('Gearbox')
analysis_main.add_module(gear)
geom = register_module('Geometry')
# geom.param("components", components)
analysis_main.add_module(geom)
full = register_module('FullSim')
full.param('StoreAllSecondaries', True)
analysis_main.add_module(full)

add_simulation(analysis_main, components, bkgFiles)

add_mdst_output(analysis_main, True, 'exampleInput.root',
                ['CDCHits', 'PixelClusters', 'SVDClusters', 'CDCRecoHits', 'SVDRecoHits', 'PXDRecoHits',
                 'SVDSimHits', 'PXDSimHits', 'CDCSimHits', 'PXDClusters'])

# Show progress of processing
progress = register_module('Progress')
analysis_main.add_module(progress)

process(analysis_main)
print statistics
