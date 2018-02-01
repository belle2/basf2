#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
import numpy as np
import os
import sys
import glob
from ROOT import gROOT, Belle2
gROOT.ProcessLine("gErrorIgnoreLevel = 4000;")  # ignore endless root errors for background files...

"""
Compare the CDCTrigger2DFitterModule output with the old TRGCDCModule,
to confirm that the behaviour of the new code is correct.
"""

# ------------ #
# user options #
# ------------ #

# general options
seed = 1
evtnum = 1000
clock = False
particlegun_params = {
    'pdgCodes': [-13, 13],
    'nTracks': 2,
    'momentumGeneration': 'inversePt',
    'momentumParams': [0.3, 10.],
    'thetaGeneration': 'uniform',
    'thetaParams': [35, 123],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'fixed',
    'xVertexParams': [0.],
    'yVertexParams': [0.],
    'zVertexParams': [0.]}
usebkg = False
bkgdir = '/sw/belle2/bkg/'

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
if usebkg:
    bkgmixer = basf2.register_module('BeamBkgMixer')
    bkgfiles = glob.glob(os.path.join(bkgdir, '*[!(PXD)(ECL)]??.root'))
    bkgmixer.param('backgroundFiles', bkgfiles)
    bkgmixer.param('components', ['CDC'])
    main.add_module(bkgmixer)
cdcdigitizer = basf2.register_module('CDCDigitizer')
if clock:
    cdcdigitizer.param('TrigTimeJitter', 32.)
main.add_module(cdcdigitizer)

# ----------- #
# CDC trigger #
# ----------- #

trgcdc = basf2.register_module('TRGCDC')
simMode = 0  # 0: full trigger, 1: only TSF
if clock:
    simMode += 2  # 2: full trigger (TSF with clock), 3: only TSF with clock
trgcdc_params = {
    'ConfigFile': Belle2.FileSystem.findFile("data/trg/cdc/TRGCDCConfig_0_20101111.dat"),
    'InnerTSLUTFile': Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_v3.0.coe"),
    'OuterTSLUTFile': Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_v3.0.coe"),
    'SimulationMode': 1,  # only fast simulation
    'FastSimulationMode': simMode,
    'HoughFinderMappingFileMinus': Belle2.FileSystem.findFile("data/trg/cdc/HoughMappingMinus20160223.dat"),
    'HoughFinderMappingFilePlus': Belle2.FileSystem.findFile("data/trg/cdc/HoughMappingPlus20160223.dat"),
    'DebugLevel': 0}
trgcdc.param(trgcdc_params)
if clock:
    trgcdc.param('inputCollection', 'CDCHits4Trg')
main.add_module(trgcdc)

# ETF
main.add_module('CDCTriggerETF')


# ----------- #
# test module #
# ----------- #

class TestModule(basf2.Module):
    """
    test module to compare the output of TRGCDC and CDCTriggerETF
    """

    def event(self):
        """
        give info for both modules and warnings in the case of mismatches
        """
        oldT0 = Belle2.PyStoreObj("CDCTriggerEventTime").obj().getTiming()
        if Belle2.PyStoreObj("BinnedEventT0").hasBinnedEventT0(Belle2.Const.CDC):
            newT0 = Belle2.PyStoreObj("BinnedEventT0").obj().getBinnedEventT0(Belle2.Const.CDC)
        else:
            newT0 = 9999
        if oldT0 == newT0:
            basf2.B2INFO("T0 %d" % oldT0)
        else:
            basf2.B2WARNING("old T0 %d, new T0 %d" % (oldT0, newT0))


main.add_module(TestModule(), logLevel=basf2.LogLevel.INFO)

# Process events
basf2.process(main)

# Print call statistics
print(basf2.statistics)
