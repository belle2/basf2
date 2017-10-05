#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

from svd import add_svd_reconstruction
from pxd import add_pxd_reconstruction

from tracking import (
    add_mc_tracking_reconstruction,
    add_tracking_reconstruction,
    add_cr_tracking_reconstruction,
    add_mc_track_finding,
    add_track_finding,
    add_prune_tracks,
)

from softwaretrigger import (
    add_fast_reco_software_trigger,
    add_hlt_software_trigger,
    add_calibration_software_trigger,
)

import mdst


def add_reconstruction(path, components=None, pruneTracks=True, trigger_mode="all", skipGeometryAdding=False,
                       additionalTrackFitHypotheses=None, addClusterExpertModules=True, use_vxdtf2=False,
                       use_second_cdc_hits=False):
    """
    This function adds the standard reconstruction modules to a path.
    Consists of tracking and the functionality provided by :func:`add_posttracking_reconstruction()`,
    plus the modules to calculate the software trigger cuts.

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
    :param skipGeometryAdding: Advances flag: The tracking modules need the geometry module and will add it,
        if it is not already present in the path. In a setup with multiple (conditional) paths however, it can not
        determine, if the geometry is already loaded. This flag can be used to just turn off the geometry adding at
        all (but you will have to add it on your own then).
    :param additionalTrackFitHypotheses: Change the additional fitted track fit hypotheses. If no argument is given,
        the additional fitted hypotheses are muon, kaon and proton, i.e. [13, 321, 2212].
    :param addClusterExpertModules: Add the cluster expert modules in the KLM and ECL. Turn this off to reduce
        execution time.
    :param use_vxdtf2: if true the VXDTF version 2 will be used if false (default) verion 1 of the VXDTF will be used.
    :param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.
    """

    # add svd_reconstruction
    if components is None or 'SVD' in components:
        add_svd_reconstruction(path)

    # add pxd_reconstruction
    if components is None or 'PXD' in components:
        add_pxd_reconstruction(path)

    # Add tracking reconstruction modules
    add_tracking_reconstruction(path,
                                components=components,
                                pruneTracks=False,
                                mcTrackFinding=False,
                                trigger_mode=trigger_mode,
                                skipGeometryAdding=skipGeometryAdding,
                                additionalTrackFitHypotheses=additionalTrackFitHypotheses,
                                use_vxdtf2=use_vxdtf2,
                                use_second_cdc_hits=use_second_cdc_hits)

    # Statistics summary
    path.add_module('StatisticsSummary').set_name('Sum_Tracking')

    # Add further reconstruction modules
    add_posttracking_reconstruction(path,
                                    components=components,
                                    pruneTracks=pruneTracks,
                                    addClusterExpertModules=addClusterExpertModules,
                                    trigger_mode=trigger_mode)

    # Add the modules calculating the software trigger cuts (but not performing them)
    if trigger_mode == "all" and (not components or (
            "CDC" in components and "ECL" in components and "EKLM" in components and "BKLM" in components)):
        add_fast_reco_software_trigger(path)
        add_hlt_software_trigger(path)
        add_calibration_software_trigger(path)


def add_cosmics_reconstruction(
        path,
        components=None,
        pruneTracks=True,
        skipGeometryAdding=False,
        eventTimingExtraction=True,
        addClusterExpertModules=True,
        merge_tracks=True,
        top_in_counter=False,
        data_taking_period='gcr2017',
        use_second_cdc_hits=False):
    """
    This function adds the standard reconstruction modules for cosmic data to a path.
    Consists of tracking and the functionality provided by :func:`add_posttracking_reconstruction()`,
    plus the modules to calculate the software trigger cuts.

    :param path: Add the modules to this path.
    :param data_taking_period: The cosmics generation will be added using the
           parameters, that where used in this period of data taking. The periods can be found in cdc/cr/__init__.py.

    :param components: list of geometry components to include reconstruction for, or None for all components.
    :param pruneTracks: Delete all hits except the first and last of the tracks after the dEdX modules.
    :param skipGeometryAdding: Advances flag: The tracking modules need the geometry module and will add it,
        if it is not already present in the path. In a setup with multiple (conditional) paths however, it can not
        determine, if the geometry is already loaded. This flag can be used to just turn off the geometry adding at
        all (but you will have to add it on your own then).

    :param eventTimingExtraction: extract time with either the TrackTimeExtraction or
        FullGridTrackTimeExtraction modules.
    :param addClusterExpertModules: Add the cluster expert modules in the KLM and ECL. Turn this off to reduce
        execution time.

    :param merge_tracks: The upper and lower half of the tracks should be merged together in one track
    :param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.

    :param top_in_counter: time of propagation from the hit point to the PMT in the trigger counter is subtracted
           (assuming PMT is put at -z of the counter).
    """

    # Add cdc tracking reconstruction modules
    add_cr_tracking_reconstruction(path,
                                   components=components,
                                   prune_tracks=False,
                                   skip_geometry_adding=skipGeometryAdding,
                                   event_time_extraction=eventTimingExtraction,
                                   merge_tracks=merge_tracks,
                                   data_taking_period=data_taking_period,
                                   top_in_counter=top_in_counter,
                                   use_second_cdc_hits=use_second_cdc_hits)

    # Statistics summary
    path.add_module('StatisticsSummary').set_name('Sum_Tracking')

    # Add further reconstruction modules
    add_posttracking_reconstruction(path,
                                    components=components,
                                    pruneTracks=pruneTracks,
                                    addClusterExpertModules=addClusterExpertModules,
                                    trigger_mode="all")


def add_mc_reconstruction(path, components=None, pruneTracks=True, addClusterExpertModules=True,
                          use_second_cdc_hits=False):
    """
    This function adds the standard reconstruction modules with MC tracking
    to a path.

    @param components list of geometry components to include reconstruction for, or None for all components.
    @param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.
    """

    # tracking
    add_mc_tracking_reconstruction(path,
                                   components=components,
                                   pruneTracks=False,
                                   use_second_cdc_hits=use_second_cdc_hits)

    # Statistics summary
    path.add_module('StatisticsSummary').set_name('Sum_Tracking')

    # add further reconstruction modules
    add_posttracking_reconstruction(path,
                                    components=components,
                                    pruneTracks=pruneTracks,
                                    addClusterExpertModules=addClusterExpertModules)


def add_posttracking_reconstruction(path, components=None, pruneTracks=True, addClusterExpertModules=True,
                                    trigger_mode="all"):
    """
    This function adds the standard reconstruction modules after tracking
    to a path.

    :param path: The path to add the modules to.
    :param components: list of geometry components to include reconstruction for, or None for all components.
    :param pruneTracks: Delete all hits except the first and last after the dEdX modules.
    :param trigger_mode: Please see add_reconstruction for a description of all trigger modes.
    :param addClusterExpertModules: Add the cluster expert modules in the KLM and ECL. Turn this off to reduce
        execution time.
    """

    if trigger_mode in ["hlt", "all"]:
        add_dedx_modules(path, components, pruneTracks)
        add_ext_module(path, components)
        add_top_modules(path, components)
        add_arich_modules(path, components)

    path.add_module('StatisticsSummary').set_name('Sum_PID')

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

    if trigger_mode in ["all"] and addClusterExpertModules:
        # FIXME: Disabled for HLT until execution time bug is fixed
        add_cluster_expert_modules(path, components)
        # add_ecl_track_cluster_module(path, components)

    path.add_module('StatisticsSummary').set_name('Sum_Clustering')


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

    return mdst.add_mdst_output(path, mc, filename, additionalBranches, dataDescription)


def add_arich_modules(path, components=None):
    """
    Add the ARICH reconstruction to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'ARICH' in components:
        arich_fillHits = register_module('ARICHFillHits')
        path.add_module(arich_fillHits)
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
        top_cm = register_module('TOPChannelMasker')
        path.add_module(top_cm)
        top_rec = register_module('TOPReconstructor')
        path.add_module(top_rec)


def add_cluster_expert_modules(path, components=None):
    """
    Add the cluster expert modules to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    # klong id and cluster matcher, whcih also builds "cluster"
    if components is None or ('EKLM' in components and 'BKLM' in components and 'ECL' in components):
        KLMClassifier = register_module('KLMExpert')
        path.add_module(KLMClassifier)


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
    if components is None or ('BKLM' in components and 'EKLM' in components):
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
        # muid.logging.log_level = LogLevel.DEBUG
        # muid.logging.debug_level = 100
        # muid.param("pdgCodes", [11, 13, 211, 321, 2212])
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

        # ECL connected region finder
        ecl_crfinder = register_module('ECLCRFinder')
        path.add_module(ecl_crfinder)

        # ECL local maximum finder
        ecl_lmfinder = register_module('ECLLocalMaximumFinder')
        path.add_module(ecl_lmfinder)

        # ECL splitter N1
        ecl_splitterN1 = register_module('ECLSplitterN1')
        path.add_module(ecl_splitterN1)

        # ECL splitter N2
        ecl_splitterN2 = register_module('ECLSplitterN2')
        path.add_module(ecl_splitterN2)

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


def add_ecl_track_cluster_module(path, components=None):
    """
    Add the ECL track cluster matching module to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'ECL' in components:
        # track cluster matching
        ecl_track_cluster = register_module('ECLTrackClusterMatching')
        ecl_track_cluster.logging.log_level = LogLevel.DEBUG
        ecl_track_cluster.logging.debug_level = 90
        path.add_module(ecl_track_cluster)


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
        # ext.logging.log_level = LogLevel.DEBUG
        # ext.logging.debug_level = 90
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
    # only run this if the SVD is enabled - PXD is disabled by default
    if components is None or 'SVD' in components:
        VXDdEdxPID = register_module('VXDDedxPID')
        path.add_module(VXDdEdxPID)

    # Prune tracks as soon as the intermediate states at each measurement are not needed anymore.
    if pruneTracks:
        add_prune_tracks(path, components)
