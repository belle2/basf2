#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
import os
import glob
from ROOT import gROOT, Belle2
gROOT.ProcessLine("gErrorIgnoreLevel = 4000;")  # ignore endless root errors for background files...

"""
Run the Track Segment Finder and save a table of true left/right for each hit pattern.
This table can be used to create the left/right LUT (see createTSLUT.py),
or to evaluate it (see evaluateTSLUT.py).
"""

# ------------ #
# user options #
# ------------ #

# general options
seed = 1
evtnum = 100000
clock = False
backgroundLevel = 0.
bkgdir = '/sw/belle2/bkg/'
particlegun_params = {
    'pdgCodes': [-13, 13],
    'nTracks': 1,
    'momentumGeneration': 'inversePt',
    'momentumParams': [0.27, 7.2],   # no curling tracks
    'thetaGeneration': 'uniformCos',
    'thetaParams': [35, 123],        # hit all super layers
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'fixed',
    'xVertexParams': [0.],
    'yVertexParams': [0.],
    'zVertexParams': [0.]}
# filenames for the generates tables
# including the seed in the filename allows to combine several runs later
innerFilename = 'innerTrueLRTable_Bkg%.1f_%d.dat' % (backgroundLevel, seed)
outerFilename = 'outerTrueLRTable_Bkg%.1f_%d.dat' % (backgroundLevel, seed)

# ------------------------- #
# create path up to trigger #
# ------------------------- #

# set random seed
basf2.set_random_seed(seed)
# suppress messages and warnings during processing:
basf2.set_log_level(basf2.LogLevel.ERROR)

main = basf2.create_path()

main.add_module('EventInfoSetter', evtNumList=evtnum)
main.add_module('Progress')
main.add_module('Gearbox')
main.add_module('Geometry', components=['CDC',
                                        'MagneticFieldConstant4LimitedRCDC'])
particlegun = basf2.register_module('ParticleGun')
particlegun.param(particlegun_params)
main.add_module(particlegun)
main.add_module('FullSim')
if backgroundLevel > 0:
    bkgmixer = basf2.register_module('BeamBkgMixer')
    bkgfiles = glob.glob(os.path.join(bkgdir, '*[!(PXD)(ECL)]??.root'))
    bkgmixer.param('backgroundFiles', bkgfiles)
    bkgmixer.param('components', ['CDC'])
    bkgmixer.param('overallScaleFactor', backgroundLevel)
    main.add_module(bkgmixer)
cdcdigitizer = basf2.register_module('CDCDigitizer')
if clock:
    cdcdigitizer.param('TrigTimeJitter', 32.)
main.add_module(cdcdigitizer)

tsf = basf2.register_module('CDCTriggerTSF')
tsf_params = {
    'InnerTSLUTFile': Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_v3.0.coe"),
    'OuterTSLUTFile': Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_v3.0.coe"),
    'ClockSimulation': clock,
    'makeTrueLRTable': True,
    'innerTrueLRTableFilename': innerFilename,
    'outerTrueLRTableFilename': outerFilename}
tsf.param(tsf_params)
if clock:
    tsf.param('CDCHitCollectionName', 'CDCHits4Trg')
main.add_module(tsf)

# Process events
basf2.process(main)

# Print call statistics
print(basf2.statistics)
