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
    'nTracks': 1,
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

xtsimple = False
fitdrift = True

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
    'DebugLevel': 0,
    'Fitter3Ds2DFitDrift': fitdrift,
    'Fitter3DsXtSimple': xtsimple,
    '2DfinderCollection': 'TRGCDC2DFinderTracks'}
trgcdc.param(trgcdc_params)
if clock:
    trgcdc.param('inputCollection', 'CDCHits4Trg')
main.add_module(trgcdc, logLevel=basf2.LogLevel.INFO)

# fitters
main.add_module('CDCTriggerETF')
main.add_module('CDCTrigger2DFitter', logLevel=basf2.LogLevel.INFO,
                minHits=2, useDriftTime=fitdrift, xtSimple=xtsimple,
                outputCollectionName='FitterTracks')  # to distinguish from old output
main.add_module('CDCTrigger3DFitter', logLevel=basf2.LogLevel.INFO,
                xtSimple=xtsimple,
                inputCollectionName='FitterTracks',
                outputCollectionName='Fitter3DTracks')  # to distinguish from old output


# ----------- #
# test module #
# ----------- #

class TestModule(basf2.Module):
    """
    test module to compare the output of TRGCDC and CDCTrigger2DFitter/CDCTrigger3DFitter
    """

    def event(self):
        """
        give info for both output lists and warnings in the case of mismatches
        """
        phiMC = Belle2.PyStoreArray("MCParticles")[0].getMomentum().Phi()
        oldTracks = Belle2.PyStoreArray("Trg3DFitterTracks")
        newTracks = Belle2.PyStoreArray("Fitter3DTracks")
        if oldTracks.getEntries() == newTracks.getEntries():
            basf2.B2INFO("%d tracks" % oldTracks.getEntries())
        else:
            basf2.B2WARNING("old version: %d, new version: %d" %
                            (oldTracks.getEntries(), newTracks.getEntries()))
        for i in range(max(oldTracks.getEntries(), newTracks.getEntries())):
            if i < oldTracks.getEntries():
                ptfactor = 0.3 * 1.5 / 100 * 222.376063
                oldString = "phi %.3f pt %.3f charge %d chi2 %.3f z %.3f cot %.3f chi2 %.3f" % \
                            (oldTracks[i].getPhi0() * 180. / np.pi,
                             oldTracks[i].getTransverseMomentum(1.5) / ptfactor,
                             oldTracks[i].getChargeSign(),
                             oldTracks[i].getChi2D(),
                             oldTracks[i].getZ0(),
                             oldTracks[i].getCotTheta(),
                             oldTracks[i].getChi3D())
            else:
                oldString = "no track"
            if i < newTracks.getEntries():
                newString = "phi %.3f pt %.3f charge %d chi2 %.3f z %.3f cot %.3f chi2 %.3f" % \
                            (newTracks[i].getPhi0() * 180. / np.pi,
                             newTracks[i].getTransverseMomentum(1.5),
                             newTracks[i].getChargeSign(),
                             newTracks[i].getChi2D(),
                             newTracks[i].getZ0(),
                             newTracks[i].getCotTheta(),
                             newTracks[i].getChi3D())
            else:
                newString = "no track"
            if oldString == newString:
                basf2.B2INFO(oldString)
            else:
                basf2.B2WARNING("old: " + oldString)
                basf2.B2WARNING("new: " + newString)


main.add_module(TestModule(), logLevel=basf2.LogLevel.INFO)

# Process events
basf2.process(main)

# Print call statistics
print(basf2.statistics)
