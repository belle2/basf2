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


def add_reconstruction(path, components=None, pruneTracks=True, trigger_mode="all", skipGeometryAdding=False):
    """
    This function adds the standard reconstruction modules to a path.
    Consists of tracking and the functionality provided by :func:`add_posttracking_reconstruction()`

    :param path: Add the modules to this path.
    :param components: list of geometry components to include reconstruction for, or None for all components.
    :param pruneTracks: Delete all hits except the first and last of the tracks after the dEdX modules.
    :param trigger_mode: Trigger mode to emulate. Possible values are: "all", "hlt", "fast_reco".

        * "all": Normal mode. Do add all the modules in the standard reconstruction. This is the mode for the typical
          user.
        * "fast_reco": Only add those modules that are needed for classifying an event as background or not in the
          fast reco part of the software trigger.
        * "hlt": Only add those modules thar are needed for classifying an event as background or not in the
          hlt part of the software trigger. Please note that the fast reco part is also needed for this to work.

        The trigger_mode does just steer, which modules in the standard reconstruction are added to the path. It does
        not make any trigger decisions itself.
    """

    # tracking
    add_tracking_reconstruction(path,
                                components=components,
                                pruneTracks=False,
                                mcTrackFinding=False,
                                trigger_mode=trigger_mode,
                                skipGeometryAdding=skipGeometryAdding)

    # add further reconstruction modules
    add_posttracking_reconstruction(path,
                                    components=components,
                                    pruneTracks=pruneTracks,
                                    trigger_mode=trigger_mode)


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


def add_posttracking_reconstruction(path, components=None, pruneTracks=True,
                                    trigger_mode="all"):
    """
    This function adds the standard reconstruction modules after tracking
    to a path.

    :param path: The path to add the modules to.
    :param components: list of geometry components to include reconstruction for, or None for all components.
    :param pruneTracks: Delete all hits except the first and last after the dEdX modules.
    :param trigger_mode: Please see add_reconstruction for a description of all trigger modes.
    """

    if trigger_mode in ["hlt", "all"]:
        add_dedx_modules(path, components, pruneTracks)
        add_ext_module(path, components)
        add_top_modules(path, components)
        add_arich_modules(path, components)

    if trigger_mode in ["fast_reco", "all"]:
        add_ecl_modules(path, components)

    if trigger_mode in ["hlt", "all"]:
        add_ecl_track_matcher_module(path, components)
        add_ecl_eip_module(path, components)

    if trigger_mode in ["hlt", "all"]:
        add_ecl_mc_matcher_module(path, components)

        add_klm_modules(path, components)

        add_klm_mc_matcher_module(path, components)

        add_muid_module(path, components)
        add_pid_module(path, components)

    if trigger_mode in ["all"]:
        # FIXME: Disabled for HLT until performance bug is fixed
        add_cluster_expert_modules(path, components)


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


def add_arich_modules(path, components=None):
    """
    Add the ARICH reconstruction to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'ARICH' in components:
        arich_rec = register_module('ARICHReconstructor')
        path.add_module(arich_rec)


def add_top_modules(path, components=None):
    """
    Add the TOP reconstruction to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    # TOP reconstruction
    if components is None or 'TOP' in components:
        top_rec = register_module('TOPReconstructor')
        path.add_module(top_rec)


def add_cluster_expert_modules(path, components=None):
    """
    Add the cluster expert modules to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    # klong id and cluster matcher, whcih also builds "cluster"
    if components is None or 'EKLM' and 'BKLM' and 'ECL' in components:
        KLMClassifier = register_module('KLMExpert')
        path.add_module(KLMClassifier)

        ECLClassifier = register_module('ECLExpert')
        path.add_module(ECLClassifier)

        ClusterMatcher = register_module('ClusterMatcher')
        path.add_module(ClusterMatcher)


def add_pid_module(path, components=None):
    """
    Add the PID modules to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    # charged particle PID
    if components is None or 'SVD' in components or 'CDC' in components:
        mdstPID = register_module('MdstPID')
        path.add_module(mdstPID)


def add_klm_modules(path, components=None):
    """
    Add the (E/B)KLM reconstruction modules to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'EKLM' in components:
        eklm_rec = register_module('EKLMReconstructor')
        path.add_module(eklm_rec)

    # BKLM reconstruction
    if components is None or 'BKLM' in components:
        bklm_rec = register_module('BKLMReconstructor')
        path.add_module(bklm_rec)

    # K0L reconstruction
    if components is None or 'BKLM' in components or 'EKLM' in components:
        klm_k0l_rec = register_module('KLMK0LReconstructor')
        path.add_module(klm_k0l_rec)


def add_klm_mc_matcher_module(path, components=None):
    """
    Add the (E/B)KLM mc matcher module to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    # MC matching
    if components is None or 'BKLM' in components or 'EKLM' in components:
        klm_mc = register_module('MCMatcherKLMClusters')
        path.add_module(klm_mc)


def add_muid_module(path, components=None):
    """
    Add the MuID module to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'BKLM' in components and 'EKLM' in components:
        muid = register_module('Muid')
        path.add_module(muid)


def add_ecl_modules(path, components=None):
    """
    Add the ECL reconstruction modules to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
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


def add_ecl_track_matcher_module(path, components=None):
    """
    Add the ECL track matcher module to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'ECL' in components:
        # track shower matching
        ecl_track_match = register_module('ECLTrackShowerMatch')
        path.add_module(ecl_track_match)


def add_ecl_eip_module(path, components=None):
    """
    Add the ECL electron ID module to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'ECL' in components:
        # electron ID
        electron_id = register_module('ECLElectronId')
        path.add_module(electron_id)


def add_ecl_mc_matcher_module(path, components=None):
    """
    Add the ECL MC matcher module to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'ECL' in components:
        # MC matching
        ecl_mc = register_module('MCMatcherECLClusters')
        path.add_module(ecl_mc)


def add_ext_module(path, components=None):
    """
    Add the extrapolation module to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'CDC' in components:
        ext = register_module('Ext')
        path.add_module(ext)


def add_dedx_modules(path, components=None, pruneTracks=True):
    """
    Add the dEdX reconstruction modules to the path
    and prune the tracks afterwards if wanted.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    :param pruneTracks: delete all hits except the first or last hit in the tracks.
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
