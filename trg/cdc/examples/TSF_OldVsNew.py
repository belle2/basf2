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
Compare the CDCTriggerTSFModule output with the old TRGCDCModule,
to confirm that the behaviour of the new code is correct.
"""

# ------------ #
# user options #
# ------------ #

# general options
seed = 1
evtnum = 100
clock = True
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

trgcdc = basf2.register_module('TRGCDC')
simMode = 1  # 0: full trigger, 1: only TSF
if clock:
    simMode += 2  # 2: full trigger (TSF with clock), 3: only TSF with clock
trgcdc_params = {
    'ConfigFile': Belle2.FileSystem.findFile("data/trg/cdc/TRGCDCConfig_0_20101111.dat"),
    'InnerTSLUTFile': Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_v3.0.coe"),
    'OuterTSLUTFile': Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_v3.0.coe"),
    'SimulationMode': 1,  # only fast simulation
    'FastSimulationMode': simMode,
    'HoughFinderMappingFileMinus': Belle2.FileSystem.findFile("data/trg/cdc/HoughMappingMinus20160223.dat"),
    'HoughFinderMappingFilePlus': Belle2.FileSystem.findFile("data/trg/cdc/HoughMappingPlus20160223.dat")}
trgcdc.param(trgcdc_params)
if clock:
    trgcdc.param('inputCollection', 'CDCHits4Trg')
main.add_module(trgcdc)

tsf = basf2.register_module('CDCTriggerTSF')
tsf_params = {
    'InnerTSLUTFile': Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_v3.0.coe"),
    'OuterTSLUTFile': Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_v3.0.coe"),
    'TSHitCollectionName': 'TSHits',  # to distinguish from old output
    'ClockSimulation': clock}
tsf.param(tsf_params)
if clock:
    tsf.param('CDCHitCollectionName', 'CDCHits4Trg')
main.add_module(tsf)


# ----------- #
# test module #
# ----------- #

class TestModule(basf2.Module):
    """
    test module to compare the output of TRGCDC and CDCTriggerTSF
    """

    def event(self):
        """
        give info for both output lists and warnings in the case of mismatches
        """
        oldHits = Belle2.PyStoreArray("CDCTriggerSegmentHits")
        newHits = Belle2.PyStoreArray("TSHits")
        if oldHits.getEntries() == newHits.getEntries():
            basf2.B2INFO("%d hits" % oldHits.getEntries())
        else:
            basf2.B2WARNING("old version: %d, new version: %d" %
                            (oldHits.getEntries(), newHits.getEntries()))
        for i in range(max(oldHits.getEntries(), newHits.getEntries())):
            if i < oldHits.getEntries():
                oldString = "ID %d priority %d LR %d fastest T %d priority T %d found T %d" % \
                            (oldHits[i].getSegmentID(), oldHits[i].getPriorityPosition(),
                             oldHits[i].getLeftRight(), oldHits[i].fastestTime(),
                             oldHits[i].priorityTime(), oldHits[i].foundTime())
            else:
                oldString = "no hit"
            if i < newHits.getEntries():
                newString = "ID %d priority %d LR %d fastest T %d priority T %d found T %d" % \
                            (newHits[i].getSegmentID(), newHits[i].getPriorityPosition(),
                             newHits[i].getLeftRight(), newHits[i].fastestTime(),
                             newHits[i].priorityTime(), newHits[i].foundTime())
            else:
                newString = "no hit"
            if oldString == newString:
                basf2.B2INFO(oldString)
            else:
                basf2.B2WARNING("old: " + oldString)
                basf2.B2WARNING("new: " + newString)
            # check relations
            if clock:
                oldCDCRels = oldHits[i].getRelationsTo("CDCHits4Trg")
                newCDCRels = newHits[i].getRelationsTo("CDCHits4Trg")
            else:
                oldCDCRels = oldHits[i].getRelationsTo("CDCHits")
                newCDCRels = newHits[i].getRelationsTo("CDCHits")
            if len(oldCDCRels) == len(newCDCRels):
                basf2.B2INFO("%d related CDCHits" % len(oldCDCRels))
            else:
                basf2.B2WARNING("old version: %d related CDCHits" % len(oldCDCRels))
                basf2.B2WARNING("new version: %d related CDCHits" % len(newCDCRels))
            for irel in range(max(len(oldCDCRels), len(newCDCRels))):
                if irel < len(oldCDCRels):
                    oldString = "relation to hit %d, weight %.1f" % \
                                (oldCDCRels[irel].getArrayIndex(), oldCDCRels.weight(irel))
                else:
                    oldString = "no relation"
                if irel < len(newCDCRels):
                    newString = "relation to hit %d, weight %.1f" % \
                                (newCDCRels[irel].getArrayIndex(), newCDCRels.weight(irel))
                else:
                    newString = "no relation"
                if oldString == newString:
                    basf2.B2INFO(oldString)
                else:
                    basf2.B2WARNING(oldString)
                    basf2.B2WARNING(newString)


main.add_module(TestModule(), logLevel=basf2.LogLevel.INFO)

# Process events
basf2.process(main)

# Print call statistics
print(basf2.statistics)
