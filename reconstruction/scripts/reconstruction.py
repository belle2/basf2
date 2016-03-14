#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

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
        add_prune_tracks(path)

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

        # digit calibration
        ecl_digit_calibration = register_module('ECLDigitCalibrator')
        path.add_module(ecl_digit_calibration)

        # shower reconstruction
        ecl_shower_rec = register_module('ECLReconstructor')
        path.add_module(ecl_shower_rec)

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


def add_reconstruction(path, components=None, pruneTracks=True):
    """
    This function adds the standard reconstruction modules to a path.
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
    This function adds the mdst output modules to a path.
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
    if mc:
        branches += ['MCParticles', 'TracksToMCParticles',
                     'ECLClustersToMCParticles', 'KLMClustersToMCParticles']
    branches += additionalBranches
    output.param('branchNames', branches)
    output.param('branchNamesPersistent', ['FileMetaData'])
    path.add_module(output)
