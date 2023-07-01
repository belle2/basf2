#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
import os
import glob
from ROOT import gROOT, Belle2
import reconstruction
import simulation

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
evtnum = 1000  # 00
clock = False
backgroundLevel = 0.
bkgdir = '/remote/neurobelle/bkg/'
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
innerFilename = f'innerTrueLRTable_Bkg{backgroundLevel:.1f}_{int(seed)}.dat'
outerFilename = f'outerTrueLRTable_Bkg{backgroundLevel:.1f}_{int(seed)}.dat'
innerrecoFilename = f'innerRecoLRTable_Bkg{backgroundLevel:.1f}_{int(seed)}.dat'
outerrecoFilename = f'outerRecoLRTable_Bkg{backgroundLevel:.1f}_{int(seed)}.dat'

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
main.add_module('Geometry')  # , components=['CDC',
#            'MagneticFieldConstant4LimitedRCDC'])
particlegun = basf2.register_module('ParticleGun')
particlegun.param(particlegun_params)
main.add_module(particlegun)
simulation.add_simulation(main)
# main.add_module('FullSim')
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
reconstruction.add_reconstruction(main)
tsf = basf2.register_module('CDCTriggerTSF')
tsf_params = {
    'InnerTSLUTFile': Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_v3.0.coe"),
    'OuterTSLUTFile': Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_v3.0.coe"),
    'ClockSimulation': clock,
    'makeTrueLRTable': True,
    'makeRecoLRTable': True,
    'innerTrueLRTableFilename': innerFilename,
    'outerTrueLRTableFilename': outerFilename,
    'innerRecoLRTableFilename': innerrecoFilename,
    'outerRecoLRTableFilename': outerrecoFilename}
tsf.param(tsf_params)
if clock:
    tsf.param('CDCHitCollectionName', 'CDCHits4Trg')
main.add_module(tsf)

# Process events
basf2.process(main)

# Print call statistics
print(basf2.statistics)
