#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
#
# Example Script to use SVDVariablesToStorageModule on simulation or real data
#
##########################################################################

import basf2 as b2
import tracking as trk
import simulation as sim
import svd as svd
import glob
import modularAnalysis as ma
import rawdata as raw
from basf2 import conditions as b2conditions


def addSVDVariablesToStorageModule(path, storageType):

    if (storageType == "ntuple"):
        path.add_module('SVDVariablesToStorage',
                        outputFileName="SVDVariables_ntuple.root",
                        containerName="SVDClusters",
                        particleListName="pi+:all",
                        variablesToNtuple=["SVDClusterCharge", "SVDClusterSNR", "SVDClusterSize", "SVDClusterTime",
                                           "SVDTrackPrime", "SVDResidual", "SVDLayer", "SVDLadder", "SVDSensor", "SVDSide"]
                        ).set_log_level(b2.LogLevel.INFO)
    elif (storageType == "histogram"):
        path.add_module('SVDVariablesToStorage',
                        outputFileName="SVDVariables_histogram.root",
                        containerName="SVDClusters",
                        particleListName="pi+:all",
                        variablesToHistogram=[("SVDClusterCharge", 100, 0, 100e3),
                                              ("SVDClusterSNR", 50, 0, 50),
                                              ("SVDClusterSize", 10, 0, 10),
                                              ("SVDClusterTime", 100, -100, 100),
                                              ("SVDTrackPrime", 25, -1, 1),
                                              ("SVDResidual", 100, -1e5, 1e5),
                                              ("SVDLayer", 4, 3, 7),
                                              ("SVDLadder", 16, 1, 17),
                                              ("SVDSensor", 5, 1, 6),
                                              ("SVDSide", 2, 0, 2)]
                        ).set_log_level(b2.LogLevel.INFO)


if __name__ == '__main__':

    useSimulation = True

    b2.set_log_level(b2.LogLevel.ERROR)
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
        b2conditions.reset()
        b2conditions.override_globaltags()
        b2conditions.globaltags = ["online"]

        # input root files
        main.add_module('RootInput', branchNames=['RawPXDs', 'RawSVDs', 'RawCDCs'])
        raw.add_unpackers(main, components=['PXD', 'SVD', 'CDC'])

    # now do reconstruction:
    trk.add_tracking_reconstruction(
        main,
        mcTrackFinding=MCTracking,
        trackFitHypotheses=[211],
        append_full_grid_cdc_eventt0=True,
        skip_full_grid_cdc_eventt0_if_svd_time_present=False)

    # Reconstruct strips
    svd.add_svd_create_recodigits(main)

    # Fill particle lists
    ma.fillParticleLists(decayStringsWithCuts=[("pi+:all", "")], path=main)

    # Add SVDVariablesToStorageModule
    addSVDVariablesToStorageModule(main, "ntuple")
    # addSVDVariablesToStorageModule(main, "histogram")

    main.add_module('RootOutput', branchNames=[
        'EventExtraInfo',
        'EventMetaData',
        'Particles',
        'ParticlesToTrackFitResults',
        'RecoHitInformations',
        'RecoHitInformationsToSVDClusters',
        'RecoTracks',
        'SVDClusters',
        'SVDClustersToRecoTracks',
        'SVDClustersToSVDTrueHits',
        'SVDRecoTracks',
        'SVDSimHits',
        'SVDTrueHits',
        'TrackFitResults',
        'Tracks',
        'TracksToRecoTracks',
    ])

    # Summary
    main.add_module('Progress')
    b2.print_path(main)
    b2.process(main)
