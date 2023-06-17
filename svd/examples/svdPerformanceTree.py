#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##################################################################################
#
# Example Script to use SVDPerformanceTTree and OverlapResidual Modules
# on simulation or real data
#
###################################################################################

import basf2 as b2
# from basf2 import conditions as b2conditions
import rawdata as raw
import tracking as trk
import simulation as sim
import svd as svd
import glob

useSimulation = True

# set this string to identify the output rootfiles
tag = "_test"

main = b2.create_path()

b2.set_random_seed(1)

if useSimulation:
    # options for simulation:
    expList = [1003]
    numEvents = 2000
    bkgFiles = glob.glob('/sw/belle2/bkg/*.root')  # Phase3 background
    bkgFiles = None  # uncomment to remove  background
    simulateJitter = False
    ROIfinding = False
    MCTracking = False
    eventinfosetter = b2.register_module('EventInfoSetter')
    eventinfosetter.param('expList', expList)
    eventinfosetter.param('runList', [0])
    eventinfosetter.param('evtNumList', [numEvents])
    main.add_module(eventinfosetter)
    # main.add_module('EventInfoPrinter')
    main.add_module('EvtGenInput')

    sim.add_simulation(
        main,
        bkgfiles=bkgFiles,
        forceSetPXDDataReduction=True,
        usePXDDataReduction=ROIfinding,
        simulateT0jitter=simulateJitter)
else:
    MCTracking = False

    # setup database - if needed
    # b2conditions.reset()
    # b2conditions.override_globaltags()
    # b2conditions.globaltags = ["online"]

    # input root files
    main.add_module('RootInput', branchNames=['RawPXDs', 'RawSVDs', 'RawCDCs'])
    raw.add_unpackers(main, components=['PXD', 'SVD', 'CDC'])

    # change ZS to 5 - if needed
    # for moda in main.modules():
    #    if moda.name() == 'SVDUnpacker':
    #        moda.param("svdShaperDigitListName", "SVDShaperDigitsZS3")
    # main.add_module("SVDZeroSuppressionEmulator",SNthreshold=5,ShaperDigits="SVDShaperDigitsZS3",ShaperDigitsIN="SVDShaperDigits")

# now do reconstruction:
trk.add_tracking_reconstruction(
    main,
    mcTrackFinding=MCTracking,
    trackFitHypotheses=[211],
    append_full_grid_cdc_eventt0=True)

# reconstruct strips
svd.add_svd_create_recodigits(main)

# look at raw time - uncomment if needed
b2.set_module_parameters(main, "SVDClusterizer", returnClusterRawTime=True)

# fill TTrees
main.add_module('SVDPerformanceTTree', outputFileName="SVDPerformanceTree"+str(tag)+".root")
main.add_module('OverlapResiduals', ExpertLevel=True)

main.add_module('Progress')

b2.print_path(main)

b2.process(main)

print(b2.statistics)
