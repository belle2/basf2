#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *


def add_mdst_output(
    path,
    mc=True,
    filename='mdst.root',
    additionalBranches=[],
    dataDescription=None,
):
    """
    This function adds the MDST output modules to a path, saving only objects defined as part of the MDST data format.

    @param path Path to add modules to
    @param mc Save Monte Carlo quantities? (MCParticles and corresponding relations)
    @param filename Output file name.
    @param additionalBranches Additional objects/arrays of event durability to save
    @param dataDescription Additional key->value pairs to be added as data description
           fields to the output FileMetaData
    """

    output = register_module('RootOutput')
    output.param('outputFileName', filename)
    branches = [
        'Tracks',
        'V0s',
        'TrackFitResults',
        'PIDLikelihoods',
        'TracksToPIDLikelihoods',
        'ECLClusters',
        'TracksToECLClusters',
        'KLMClusters',
        'KLMClustersToTracks',
        'KlIds',
        'KLMClustersToKlIds',
        'TRGSummary',
        'SoftwareTriggerResult',
    ]
    persistentBranches = ['FileMetaData']
    if mc:
        branches += ['MCParticles', 'TracksToMCParticles',
                     'ECLClustersToMCParticles', 'KLMClustersToMCParticles']
        persistentBranches += ['BackgroundInfo']
    branches += additionalBranches
    output.param('branchNames', branches)
    output.param('branchNamesPersistent', persistentBranches)
    # set dataDescription correctly
    if dataDescription is None:
        dataDescription = {}
    # set dataLevel to mdst if it's not already set to something else (which
    # might happen for udst output since that calls this function)
    dataDescription.setdefault("dataLevel", "mdst")
    output.param("additionalDataDescription", dataDescription)
    path.add_module(output)
    return output
