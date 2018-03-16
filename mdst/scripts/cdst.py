#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from mdst import add_mdst_output


def add_cdst_output(
    path,
    mc=True,
    filename='cdst.root',
    additionalBranches=[],
    dataDescription=None,
):
    """
    This function adds the cDST output modules (mDST + calibration objects) to a path,
    saving only objects defined as part of the cDST data format.

    @param path Path to add modules to
    @param mc Save Monte Carlo quantities? (MCParticles and corresponding relations)
    @param filename Output file name.
    @param additionalBranches Additional objects/arrays of event durability to save
    @param dataDescription Additional key->value pairs to be added as data description
           fields to the output FileMetaData
    """

    calibrationBranches = [
        'RecoTracks',
        'EventT0',
        'SVDShaperDigits',
        'CDCDedxTracks',
        'TOPDigits',
        'ExtHits',
        'TOPLikelihoods',
        'ECLDigits',
        'ECLCalDigits',
        'RecoTracks',
        'ECLShowers',
        'ECLTrgClusters',
        'BKLMHit2ds',
        'TracksToBKLMHit2ds',
        'RecoHitInformations',
        'RecoHitInformationsToBKLMHit2ds',
        'EKLMAlignmentHits',
        'EKLMHit2ds',
        'EKLMDigits',
        'ExtHits',
    ]
    if dataDescription is None:
        dataDescription = {}
    dataDescription.setdefault("dataLevel", "cdst")
    mdst.add_mdst_output(path, mc, filename, additionalBranches + calibrationBranches, dataDescription)
