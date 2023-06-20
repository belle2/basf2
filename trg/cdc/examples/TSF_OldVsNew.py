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
            basf2.B2INFO(f"{int(oldHits.getEntries())} hits")
        else:
            basf2.B2WARNING(f"old version: {int(oldHits.getEntries())}, new version: {int(newHits.getEntries())}")
        for i in range(max(oldHits.getEntries(), newHits.getEntries())):
            if i < oldHits.getEntries():
                oldString = f"ID {oldHits[i].getSegmentID():d} priority {oldHits[i].getPriorityPosition():d} LR " + \
                    f"{oldHits[i].getLeftRight():d} fastest T {oldHits[i].fastestTime():d} " + \
                    f"priority T {oldHits[i].priorityTime():d} found T {oldHits[i].foundTime():d}"
            else:
                oldString = "no hit"
            if i < newHits.getEntries():
                newString = f"ID {newHits[i].getSegmentID():d} priority {newHits[i].getPriorityPosition():d} " + \
                    f"LR {newHits[i].getLeftRight():d} fastest T {newHits[i].fastestTime():d} " + \
                    f"priority T {newHits[i].priorityTime():d} found T {newHits[i].foundTime():d}"
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
                basf2.B2INFO(f"{len(oldCDCRels)} related CDCHits")
            else:
                basf2.B2WARNING(f"old version: {len(oldCDCRels)} related CDCHits")
                basf2.B2WARNING(f"new version: {len(newCDCRels)} related CDCHits")
            for irel in range(max(len(oldCDCRels), len(newCDCRels))):
                if irel < len(oldCDCRels):
                    oldString = f"relation to hit {int(oldCDCRels[irel].getArrayIndex())}, weight {oldCDCRels.weight(irel):.1f}"
                else:
                    oldString = "no relation"
                if irel < len(newCDCRels):
                    newString = f"relation to hit {int(newCDCRels[irel].getArrayIndex())}, weight {newCDCRels.weight(irel):.1f}"
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
