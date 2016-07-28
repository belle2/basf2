#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

from tracking import (
    add_mc_tracking_reconstruction,
    add_tracking_reconstruction,
    add_mc_track_finding,
    add_track_finding,
    add_prune_tracks,
)


def add_posttracking_reconstruction(path, components=None, pruneTracks=True):
    """
    This function adds the standard reconstruction modules after tracking
    to a path.

    @param components list of geometry components to include reconstruction for, or None for all components.
    """
    # CDC dE/dx PID
    if components is None or 'CDC' in components:
        CDCdEdxPID = register_module('CDCDedxPID')
        path.add_module(CDCdEdxPID)

    # VXD dE/dx PID
    if components is None or 'SVD' in components or 'PXD' in components:
        VXDdEdxPID = register_module('VXDDedxPID')
        path.add_module(VXDdEdxPID)

    # Prune tracks as soon as the intermediate states at each measurement are not needed anymore.
    if pruneTracks:
        add_prune_tracks(path, components)

    # track extrapolation
    if components is None or 'CDC' in components:
        ext = register_module('Ext')
        path.add_module(ext)

    # TOP reconstruction
    if components is None or 'TOP' in components:
        top_rec = register_module('TOPReconstructor')
        path.add_module(top_rec)

    # ARICH reconstruction
    if components is None or 'ARICH' in components:
        arich_rec = register_module('ARICHReconstructor')
        path.add_module(arich_rec)

    # ECL calibration and reconstruction
    if components is None or 'ECL' in components:

        # ECL digit calibration
        ecl_digit_calibration = register_module('ECLDigitCalibrator')
        path.add_module(ecl_digit_calibration)

        # ECL CR finder and splitter (refactored old version - two steps at once)
        ecl_finderandsplitter = register_module('ECLCRFinderAndSplitter')
        path.add_module(ecl_finderandsplitter)

        # ECL Shower Correction
        ecl_showercorrection = register_module('ECLShowerCorrector')
        path.add_module(ecl_showercorrection)

        # ECL Shower Calibration
        ecl_showercalibration = register_module('ECLShowerCalibrator')
        path.add_module(ecl_showercalibration)

        # ECL Shower Shape
        ecl_showershape = register_module('ECLShowerShape')
        path.add_module(ecl_showershape)

        # ECL covariance matrix
        ecl_covariance = register_module('ECLCovarianceMatrix')
        path.add_module(ecl_covariance)

        # ECL finalize
        ecl_finalize = register_module('ECLFinalizer')
        path.add_module(ecl_finalize)

        # track shower matching
        ecl_track_match = register_module('ECLTrackShowerMatch')
        path.add_module(ecl_track_match)

        # electron ID
        electron_id = register_module('ECLElectronId')
        path.add_module(electron_id)

        # MC matching
        ecl_mc = register_module('MCMatcherECLClusters')
        path.add_module(ecl_mc)

    # EKLM reconstruction
    if components is None or 'EKLM' in components:
        eklm_rec = register_module('EKLMReconstructor')
        path.add_module(eklm_rec)

        # K0L reconstruction
        eklm_k0l_rec = register_module('EKLMK0LReconstructor')
        path.add_module(eklm_k0l_rec)

    # BKLM reconstruction
    if components is None or 'BKLM' in components:
        bklm_rec = register_module('BKLMReconstructor')
        path.add_module(bklm_rec)

        # K0L reconstruction
        bklm_k0l_rec = register_module('BKLMK0LReconstructor')
        path.add_module(bklm_k0l_rec)

    # MC matching
    if components is None or 'BKLM' in components or 'EKLM' in components:
        klm_mc = register_module('MCMatcherKLMClusters')
        path.add_module(klm_mc)

    # muon identification
    if components is None or 'BKLM' in components and 'EKLM' in components:
        muid = register_module('Muid')
        path.add_module(muid)

    # charged particle PID
    if components is None or 'SVD' in components or 'CDC' in components:
        mdstPID = register_module('MdstPID')
        path.add_module(mdstPID)

    # klong id and cluster matcher, whcih also builds "cluster"
    if components is None or 'EKLM' and 'BKLM' and 'ECL' in components:
        KLMClassifier = register_module('KLMExpert')
        path.add_module(KLMClassifier)

        ECLClassifier = register_module('ECLExpert')
        path.add_module(ECLClassifier)

        ClusterMatcher = register_module('ClusterMatcher')
        path.add_module(ClusterMatcher)


def add_reconstruction(path, components=None, pruneTracks=True):
    """
    This function adds the standard reconstruction modules to a path.
    Consists of tracking and the functionality provided by :func:`add_posttracking_reconstruction()`

    @param components list of geometry components to include reconstruction for, or None for all components.
    """

    # tracking
    add_tracking_reconstruction(path,
                                components=components,
                                pruneTracks=False)

    # add further reconstruction modules
    add_posttracking_reconstruction(path,
                                    components=components,
                                    pruneTracks=pruneTracks)


def add_mc_reconstruction(path, components=None, pruneTracks=True):
    """
    This function adds the standard reconstruction modules with MC tracking
    to a path.

    @param components list of geometry components to include reconstruction for, or None for all components.
    """

    # tracking
    add_mc_tracking_reconstruction(path,
                                   components=components,
                                   pruneTracks=False)

    # add further reconstruction modules
    add_posttracking_reconstruction(path,
                                    components=components,
                                    pruneTracks=pruneTracks)


def add_mdst_output(
    path,
    mc=True,
    filename='mdst.root',
    additionalBranches=[],
):
    """
    This function adds the MDST output modules to a path, saving only objects defined as part of the MDST data format.

    @param path Path to add modules to
    @param mc Save Monte Carlo quantities? (MCParticles and corresponding relations)
    @param filename Output file name.
    @param additionalBranches Additional objects/arrays of event durability to save
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
        'ECLClustersToTracks',
        'KLMClusters',
        'KLMClustersToTracks',
        'TRGSummary',
    ]
    persistentBranches = ['FileMetaData']
    if mc:
        branches += ['MCParticles', 'TracksToMCParticles',
                     'ECLClustersToMCParticles', 'KLMClustersToMCParticles']
        persistentBranches += ['BackgroundInfos']
    branches += additionalBranches
    output.param('branchNames', branches)
    output.param('branchNamesPersistent', persistentBranches)
    path.add_module(output)
