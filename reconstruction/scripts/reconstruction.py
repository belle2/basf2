#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2

from ROOT import Belle2

from geometry import check_components

from svd import add_svd_reconstruction
from pxd import add_pxd_reconstruction

from rawdata import add_unpackers

from softwaretrigger.constants import ALWAYS_SAVE_OBJECTS, RAWDATA_OBJECTS

from tracking import (
    add_mc_tracking_reconstruction,
    add_tracking_reconstruction,
    add_prefilter_tracking_reconstruction,
    add_postfilter_tracking_reconstruction,
    add_cr_tracking_reconstruction,
    add_mc_track_finding,
    add_track_finding,
    add_prune_tracks,
)

from softwaretrigger.path_utils import (
    add_filter_software_trigger,
    add_skim_software_trigger
)

import mdst


CDST_TRACKING_OBJECTS = (
    'EventLevelTrackingInfo',
    'RecoTracks',
    'Tracks',
    'V0s',
    'TrackFitResults',
    'EventT0',
    'CDCDedxTracks',
    'SVDShaperDigitsFromTracks',
    'PXDClustersFromTracks',
    'VXDDedxTracks',
    'CDCDedxLikelihoods',
    'VXDDedxLikelihoods'
)


DIGITS_OBJECTS = (
    'ARICHDigits',
    'CDCHits',
    'ECLDigits',
    'KLMDigits',
    'PXDDigits',
    'SVDEventInfoSim',
    'SVDShaperDigits',
    'TOPDigits'
)


def default_event_abort(module, condition, error_flag):
    """Default event abort outside of HLT: Ignore the error flag and just stop
    processing by giving an empty path"""
    p = basf2.Path()
    module.if_value(condition, p, basf2.AfterConditionPath.END)


def add_reconstruction(path, components=None, pruneTracks=True, add_trigger_calculation=True, skipGeometryAdding=False,
                       trackFitHypotheses=None, addClusterExpertModules=True,
                       use_second_cdc_hits=False, add_muid_hits=False, reconstruct_cdst=None,
                       event_abort=default_event_abort, use_random_numbers_for_hlt_prescale=True):
    """
    This function adds the standard reconstruction modules to a path.
    Consists of clustering, tracking and the PID modules essentially in this structure:

    | :func:`add_reconstruction()`
    | ├── :func:`add_prefilter_reconstruction()`
    | │ ├── :func:`add_prefilter_pretracking_reconstruction()`   : Clustering
    | │ ├── ``add_prefilter_tracking_reconstruction()``          : Tracking essential for HLT filter calculation
    | │ └── :func:`add_prefilter_posttracking_reconstruction()`  : PID and clustering
    | └── :func:`add_postfilter_reconstruction()`
    |   └── ``add_postfilter_tracking_reconstruction()``         : Rest of the tracking

    plus the modules to calculate the software trigger cuts.

    :param path: Add the modules to this path.
    :param components: list of geometry components to include reconstruction for, or None for all components.
    :param pruneTracks: Delete all hits except the first and last of the tracks after the V0Finder modules.
    :param skipGeometryAdding: Advances flag: The tracking modules need the geometry module and will add it,
        if it is not already present in the path. In a setup with multiple (conditional) paths however, it can not
        determine, if the geometry is already loaded. This flag can be used to just turn off the geometry adding at
        all (but you will have to add it on your own then).
    :param trackFitHypotheses: Change the additional fitted track fit hypotheses. If no argument is given,
        the fitted hypotheses are pion, muon and proton, i.e. [211, 321, 2212].
    :param addClusterExpertModules: Add the cluster expert modules in the KLM and ECL. Turn this off to reduce
        execution time.
    :param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.
    :param add_muid_hits: Add the found KLM hits to the RecoTrack. Make sure to refit the track afterwards.
    :param add_trigger_calculation: add the software trigger modules for monitoring (do not make any cut)
    :param reconstruct_cdst: None for mdst, 'rawFormat' to reconstruct cdsts in rawFormat, 'fullFormat' for the
        full (old) format. This parameter is needed when reconstructing cdsts, otherwise the
        required PXD objects won't be added.
    :param event_abort: A function to abort event processing at the given point. Should take three arguments: a module,
        the condition and the error_flag to be set if these events are kept. If run on HLT this will not abort the event
        but just remove all data except for the event information.
    :param use_random_numbers_for_hlt_prescale: If True, the HLT filter prescales are applied using randomly
        generated numbers, otherwise are applied using an internal counter.
    """

    add_prefilter_reconstruction(path,
                                 components=components,
                                 add_trigger_calculation=add_trigger_calculation,
                                 skipGeometryAdding=skipGeometryAdding,
                                 trackFitHypotheses=trackFitHypotheses,
                                 use_second_cdc_hits=use_second_cdc_hits,
                                 add_muid_hits=add_muid_hits,
                                 reconstruct_cdst=reconstruct_cdst,
                                 addClusterExpertModules=addClusterExpertModules,
                                 pruneTracks=False,
                                 event_abort=event_abort,
                                 use_random_numbers_for_hlt_prescale=use_random_numbers_for_hlt_prescale)

    # Add the modules calculating the software trigger cuts (but not performing them)
    if add_trigger_calculation and (not components or ("CDC" in components and "ECL" in components and "KLM" in components)):
        add_filter_software_trigger(path,
                                    use_random_numbers_for_prescale=use_random_numbers_for_hlt_prescale)

    add_postfilter_reconstruction(path,
                                  pruneTracks=pruneTracks,
                                  components=components)

    # Add the modules calculating the software trigger skims
    if add_trigger_calculation and (not components or ("CDC" in components and "ECL" in components and "KLM" in components)):
        add_skim_software_trigger(path)


def add_prefilter_reconstruction(path, components=None, add_trigger_calculation=True, skipGeometryAdding=False,
                                 trackFitHypotheses=None, use_second_cdc_hits=False, add_muid_hits=False,
                                 reconstruct_cdst=None, addClusterExpertModules=True, pruneTracks=True,
                                 event_abort=default_event_abort, use_random_numbers_for_hlt_prescale=True):
    """
    This function adds only the reconstruction modules required to calculate HLT filter decision to a path.
    Consists of essential tracking and the functionality provided by :func:`add_prefilter_posttracking_reconstruction()`.

    :param path: Add the modules to this path.
    :param components: list of geometry components to include reconstruction for, or None for all components.
    :param skipGeometryAdding: Advances flag: The tracking modules need the geometry module and will add it,
        if it is not already present in the path. In a setup with multiple (conditional) paths however, it can not
        determine, if the geometry is already loaded. This flag can be used to just turn off the geometry adding at
        all (but you will have to add it on your own then).
    :param trackFitHypotheses: Change the additional fitted track fit hypotheses. If no argument is given,
        the fitted hypotheses are pion, muon and proton, i.e. [211, 321, 2212].
    :param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.
    :param add_muid_hits: Add the found KLM hits to the RecoTrack. Make sure to refit the track afterwards.
    :param add_trigger_calculation: add the software trigger modules for monitoring (do not make any cut)
    :param reconstruct_cdst: None for mdst, 'rawFormat' to reconstruct cdsts in rawFormat, 'fullFormat' for the
        full (old) format. This parameter is needed when reconstructing cdsts, otherwise the
        required PXD objects won't be added.
    :param event_abort: A function to abort event processing at the given point. Should take three arguments: a module,
        the condition and the error_flag to be set if these events are kept. If run on HLT this will not abort the event
        but just remove all data except for the event information.
    :param use_random_numbers_for_hlt_prescale: If True, the HLT filter prescales are applied using randomly
        generated numbers, otherwise are applied using an internal counter.
    :param addClusterExpertModules: Add the cluster expert modules in the KLM and ECL. Turn this off to
        reduce execution time.
    :param pruneTracks: Delete all hits except the first and last of the tracks after the dEdX modules.
    """

    # Check components.
    check_components(components)

    # Do not even attempt at reconstructing events w/ abnormally large occupancy.
    doom = path.add_module("EventsOfDoomBuster")
    event_abort(doom, ">=1", Belle2.EventMetaData.c_ReconstructionAbort)
    path.add_module('StatisticsSummary').set_name('Sum_EventsofDoomBuster')

    # Add modules that have to be run BEFORE track reconstruction
    add_prefilter_pretracking_reconstruction(path,
                                             components=components)

    # Add prefilter tracking reconstruction modules
    add_prefilter_tracking_reconstruction(path,
                                          components=components,
                                          mcTrackFinding=False,
                                          skipGeometryAdding=skipGeometryAdding,
                                          trackFitHypotheses=trackFitHypotheses,
                                          use_second_cdc_hits=use_second_cdc_hits)

    # Statistics summary
    path.add_module('StatisticsSummary').set_name('Sum_Prefilter_Tracking')

    #
    # RAW CDST CASE
    #
    # If you are reconstructing a raw cdsts, add only the dE/dx calculation, PXDClustersFromTrack, SVDShaperDigitsFromTracks,
    # and pruning. Full post-tracking recon won't be run unless add_trigger_calculation is set to True.
    if reconstruct_cdst == 'rawFormat':
        # if PXD or SVD are included, you will need there two modules which are not part of the standard reconstruction
        if not components or ('PXD' in components):
            path.add_module("PXDClustersFromTracks")
        if not components or ('SVD' in components):
            path.add_module("SVDShaperDigitsFromTracks")

        # if you need to calculate the triggerResult, then you will need the full post-tracking recostruction
        if add_trigger_calculation and (not components or ("CDC" in components and "ECL" in components and "KLM" in components)):
            add_prefilter_posttracking_reconstruction(path,
                                                      components=components,
                                                      pruneTracks=pruneTracks,
                                                      add_muid_hits=add_muid_hits,
                                                      addClusterExpertModules=addClusterExpertModules)
        # if you don't need the softwareTrigger result, then you can add only these two modules of the post-tracking reconstruction
        else:
            add_dedx_modules(path)
            add_prune_tracks(path, components=components)

    #
    # FULL (aka old) CDST CASE
    #
    # if you are reconstructing a full cdst you need full post-tracking and the extra PXD and SVD modules
    elif reconstruct_cdst == 'fullFormat':
        # if PXD or SVD are included, you will need there two modules which are not part of the standard reconstruction
        if not components or ('PXD' in components):
            path.add_module("PXDClustersFromTracks")
        if not components or ('SVD' in components):
            path.add_module("SVDShaperDigitsFromTracks")

        # Add further reconstruction modules, This part is the same for mdst and full cdsts
        add_prefilter_posttracking_reconstruction(path,
                                                  components=components,
                                                  pruneTracks=pruneTracks,
                                                  add_muid_hits=add_muid_hits,
                                                  addClusterExpertModules=addClusterExpertModules)

    #
    # ANYTING ELSE CASE
    #
    # if you are not reconstucting cdsts just run the post-tracking stuff
    else:
        add_prefilter_posttracking_reconstruction(path,
                                                  components=components,
                                                  pruneTracks=pruneTracks,
                                                  add_muid_hits=add_muid_hits,
                                                  addClusterExpertModules=addClusterExpertModules)


def add_postfilter_reconstruction(path, components=None, pruneTracks=False):
    """
    This function adds the reconstruction modules not required to calculate HLT filter decision to a path.

    :param path: Add the modules to this path.
    :param components: list of geometry components to include reconstruction for, or None for all components.
    :param pruneTracks: Delete all hits expect the first and the last from the found tracks.
    """

    # Add postfilter tracking reconstruction modules
    add_postfilter_tracking_reconstruction(path, components=components, pruneTracks=pruneTracks)

    path.add_module('StatisticsSummary').set_name('Sum_Postfilter_Reconstruction')


def add_cosmics_reconstruction(
        path,
        components=None,
        pruneTracks=True,
        skipGeometryAdding=False,
        eventTimingExtraction=True,
        addClusterExpertModules=True,
        merge_tracks=True,
        use_second_cdc_hits=False,
        add_muid_hits=False,
        reconstruct_cdst=False,
        posttracking=False):
    """
    This function adds the standard reconstruction modules for cosmic data to a path.
    Consists of tracking and the functionality provided by :func:`add_prefilter_posttracking_reconstruction()`,
    plus the modules to calculate the software trigger cuts.

    :param path: Add the modules to this path.
    :param components: list of geometry components to include reconstruction for, or None for all components.
    :param pruneTracks: Delete all hits except the first and last of the tracks after the dEdX modules.
    :param skipGeometryAdding: Advances flag: The tracking modules need the geometry module and will add it,
        if it is not already present in the path. In a setup with multiple (conditional) paths however, it can not
        determine, if the geometry is already loaded. This flag can be used to just turn off the geometry adding at
        all (but you will have to add it on your own then).

    :param eventTimingExtraction: extract the event time
    :param addClusterExpertModules: Add the cluster expert modules in the KLM and ECL. Turn this off to reduce
        execution time.

    :param merge_tracks: The upper and lower half of the tracks should be merged together in one track
    :param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.

    :param add_muid_hits: Add the found KLM hits to the RecoTrack. Make sure to refit the track afterwards.

    :param reconstruct_cdst: run only the minimal reconstruction needed to produce the cdsts (raw+tracking+dE/dx)
    :param posttracking: run reconstruction for outer detectors.
    """

    # Check components.
    check_components(components)

    # Add modules that have to be run before track reconstruction
    add_prefilter_pretracking_reconstruction(path,
                                             components=components)

    # Add cdc tracking reconstruction modules
    add_cr_tracking_reconstruction(path,
                                   components=components,
                                   prune_tracks=False,
                                   skip_geometry_adding=skipGeometryAdding,
                                   event_time_extraction=eventTimingExtraction,
                                   merge_tracks=merge_tracks,
                                   use_second_cdc_hits=use_second_cdc_hits)

    # Statistics summary
    path.add_module('StatisticsSummary').set_name('Sum_Tracking')
    if posttracking:
        if reconstruct_cdst:
            # if PXD or SVD are included, you will need there two modules which are not part of the standard reconstruction
            if not components or ('PXD' in components):
                path.add_module("PXDClustersFromTracks")
            if not components or ('SVD' in components):
                path.add_module("SVDShaperDigitsFromTracks")
            # And add only the dE/dx calculation and prune the tracks
            add_dedx_modules(path)
            add_prune_tracks(path, components=components)

        else:
            # Add further reconstruction modules
            add_prefilter_posttracking_reconstruction(path,
                                                      components=components,
                                                      pruneTracks=pruneTracks,
                                                      addClusterExpertModules=addClusterExpertModules,
                                                      add_muid_hits=add_muid_hits,
                                                      cosmics=True)


def add_mc_reconstruction(path, components=None, pruneTracks=True, addClusterExpertModules=True,
                          use_second_cdc_hits=False, add_muid_hits=False):
    """
    This function adds the standard reconstruction modules with MC tracking
    to a path.

    @param components list of geometry components to include reconstruction for, or None for all components.
    @param use_second_cdc_hits: If true, the second hit information will be used in the CDC track finding.
    :param add_muid_hits: Add the found KLM hits to the RecoTrack. Make sure to refit the track afterwards.
    """

    # Add modules that have to be run before track reconstruction
    add_prefilter_pretracking_reconstruction(path,
                                             components=components)

    # tracking
    add_mc_tracking_reconstruction(path,
                                   components=components,
                                   pruneTracks=False,
                                   use_second_cdc_hits=use_second_cdc_hits)

    # Statistics summary
    path.add_module('StatisticsSummary').set_name('Sum_MC_Tracking')

    # add further reconstruction modules
    add_prefilter_posttracking_reconstruction(path,
                                              components=components,
                                              pruneTracks=pruneTracks,
                                              add_muid_hits=add_muid_hits,
                                              addClusterExpertModules=addClusterExpertModules)


def add_prefilter_pretracking_reconstruction(path, components=None):
    """
    This function adds the standard reconstruction modules BEFORE tracking
    to a path.

    :param path: The path to add the modules to.
    :param components: list of geometry components to include reconstruction for, or None for all components.
    """

    add_ecl_modules(path, components)

    # Statistics summary
    path.add_module('StatisticsSummary').set_name('Sum_Clustering')


def add_prefilter_posttracking_reconstruction(path, components=None, pruneTracks=True, addClusterExpertModules=True,
                                              add_muid_hits=False, cosmics=False, for_cdst_analysis=False,
                                              add_eventt0_combiner_for_cdst=False):
    """
    This function adds the standard reconstruction modules after tracking
    to a path.

    :param path: The path to add the modules to.
    :param components: list of geometry components to include reconstruction for, or None for all components.
    :param pruneTracks: Delete all hits except the first and last after the post-tracking modules.
    :param addClusterExpertModules: Add the cluster expert modules in the KLM and ECL. Turn this off to reduce
        execution time.
    :param add_muid_hits: Add the found KLM hits to the RecoTrack. Make sure to refit the track afterwards.
    :param cosmics: if True, steer TOP for cosmic reconstruction.
    :param for_cdst_analysis: if True, dEdx, EventT0 and PruneTracks modules are not added to the path.
           This is only needed by prepare_cdst_analysis().
    :param add_eventt0_combiner_for_cdst: if True, the EventT0Combiner module is added to the path even if
           for_cdst_analysis is False. This is useful for validation purposes for avoiding to run the full
           add_reconstruction(). Note that, with the default settings (for_cdst_analysis=False and
           add_eventt0_combiner_for_cdst=False), the EventT0Combiner module is added to the path.
    """

    # Not add dEdx modules in prepare_cdst_analysis()
    if not for_cdst_analysis:
        add_dedx_modules(path, components)

    add_ext_module(path, components)

    add_top_modules(path, components, cosmics=cosmics)

    add_arich_modules(path, components)

    # Not add EventT0Combiner module in prepare_cdst_analysis() by default,
    # but be lenient and add it if requested.
    # By default, since for_cdst_analysis is False, the module is added by this function.
    if not for_cdst_analysis or add_eventt0_combiner_for_cdst:
        path.add_module("EventT0Combiner")

    # only add the OnlineEventT0Creator if not preparing cDST
    if not for_cdst_analysis:
        path.add_module("OnlineEventT0Creator")

    add_ecl_finalizer_module(path, components)

    add_ecl_mc_matcher_module(path, components)

    add_klm_modules(path, components)

    add_klm_mc_matcher_module(path, components)

    add_muid_module(path, add_hits_to_reco_track=add_muid_hits, components=components)

    add_ecl_track_cluster_modules(path, components)

    add_ecl_cluster_properties_modules(path, components)

    add_ecl_chargedpid_module(path, components)

    add_pid_module(path, components)

    if addClusterExpertModules:
        # FIXME: Disabled for HLT until execution time bug is fixed
        add_cluster_expert_modules(path, components)

    add_ecl_track_brem_finder(path, components)

    # Prune tracks as soon as the post-tracking steps are complete
    # Not add prune tracks modules in prepare_cdst_analysis()
    if not for_cdst_analysis:
        if pruneTracks:
            add_prune_tracks(path, components)

    path.add_module('StatisticsSummary').set_name('Sum_Posttracking_Reconstruction')


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


def add_cdst_output(
    path,
    mc=True,
    filename='cdst.root',
    additionalBranches=[],
    dataDescription=None,
    rawFormat=True,
    ignoreInputModulesCheck=False
):
    """
    This function adds the `RootOutput` module to a path with the settings needed to produce a cDST output.

    @param path Path to add modules to.
    @param mc Save Monte Carlo quantities? (MCParticles and corresponding relations)
    @param filename Output file name.
    @param additionalBranches Additional objects/arrays of event durability to save
    @param dataDescription Additional key->value pairs to be added as data description
           fields to the output FileMetaData
    @param rawFormat Saves the cDST file in the raw+tracking format if mc=False, otherwise saves the cDST file
           in the digits+tracking format.
    @param ignoreInputModulesCheck If True, do not enforce check on missing PXD modules in the input path.
           Needed when a conditional path is passed as input.
    """

    branches = []
    persistentBranches = ['FileMetaData']

    if rawFormat:
        branches += list(CDST_TRACKING_OBJECTS)
        if not mc:
            branches += ALWAYS_SAVE_OBJECTS + RAWDATA_OBJECTS
        else:
            branches += list(DIGITS_OBJECTS) + [
                'EventLevelTriggerTimeInfo',
                'SoftwareTriggerResult',
                'TRGSummary']
        if not ignoreInputModulesCheck and "PXDClustersFromTracks" not in [module.name() for module in path.modules()]:
            basf2.B2ERROR("PXDClustersFromTracks is required in CDST output but its module is not found in the input path!")
    else:
        if not additionalBranches:
            basf2.B2WARNING('You are calling add_cdst_output() using rawFormat=False and requiring no additional branches. '
                            'This is equivalent to calling add_mdst_output().')
        branches += list(mdst.MDST_OBJECTS)

    if dataDescription is None:
        dataDescription = {}
    dataDescription.setdefault("dataLevel", "cdst")

    if mc:
        branches += ['MCParticles']
        persistentBranches += ['BackgroundInfo']

    branches += additionalBranches

    return path.add_module("RootOutput", outputFileName=filename, branchNames=branches,
                           branchNamesPersistent=persistentBranches, additionalDataDescription=dataDescription)


def add_arich_modules(path, components=None):
    """
    Add the ARICH reconstruction to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'ARICH' in components:
        path.add_module('ARICHFillHits')
        path.add_module('ARICHReconstructor',
                        storePhotons=1)  # enabled for ARICH DQM plots


def add_top_modules(path, components=None, cosmics=False):
    """
    Add the TOP reconstruction to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    :param cosmics: if True, steer TOP for cosmic reconstruction
    """
    if components is None or 'TOP' in components:
        path.add_module('TOPChannelMasker')
        if cosmics:
            path.add_module('TOPCosmicT0Finder')
        else:
            path.add_module('TOPBunchFinder')
        path.add_module('TOPReconstructor')


def add_cluster_expert_modules(path, components=None):
    """
    Add the KLMExpert and ClusterMatcher modules to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or ('KLM' in components and 'ECL' in components):
        path.add_module('KLMExpert')
        path.add_module('ClusterMatcher')


def add_pid_module(path, components=None):
    """
    Add the PID modules to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'SVD' in components or 'CDC' in components:
        path.add_module('MdstPID')


def add_klm_modules(path, components=None):
    """
    Add the KLM reconstruction modules to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'KLM' in components:
        path.add_module('KLMReconstructor')
        path.add_module('KLMClustersReconstructor')


def add_klm_mc_matcher_module(path, components=None):
    """
    Add the KLM mc matcher module to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'KLM' in components:
        path.add_module('MCMatcherKLMClusters')


def add_muid_module(path, add_hits_to_reco_track=False, components=None):
    """
    Add the MuID module to the path.

    :param path: The path to add the modules to.
    :param add_hits_to_reco_track: Add the found KLM hits also to the RecoTrack. Make sure to refit the track afterwards.
    :param components: The components to use or None to use all standard components.
    """
    # Muid is needed for muonID computation AND ECLCluster-Track matching.
    if components is None or ('CDC' in components and 'ECL' in components and 'KLM' in components):
        path.add_module('Muid',
                        addHitsToRecoTrack=add_hits_to_reco_track)
    if components is not None and 'CDC' in components:
        if ('ECL' not in components and 'KLM' in components):
            basf2.B2WARNING('You added KLM to the components list but not ECL: the module Muid, that is necessary '
                            'for correct muonID computation, will not be added to your reconstruction path. '
                            'Make sure that this is fine for your purposes, otherwise please include also ECL.')
        if ('ECL' in components and 'KLM' not in components):
            basf2.B2WARNING('You added ECL to the components list but not KLM: the module Muid, that is necessary '
                            'for correct ECLCluster-Track matching, will not be added to your reconstruction path. '
                            ' Make sure that this is fine for your purposes, otherwise please include also KLM.')


def add_ecl_modules(path, components=None):
    """
    Add the ECL reconstruction modules to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'ECL' in components:
        path.add_module('ECLWaveformFit')
        path.add_module('ECLDigitCalibrator')
        path.add_module('ECLEventT0')
        path.add_module('ECLCRFinder')  # connected region finder
        path.add_module('ECLLocalMaximumFinder')
        path.add_module('ECLSplitterN1')
        path.add_module('ECLSplitterN2')
        path.add_module('ECLShowerCorrector')
        path.add_module('ECLShowerCalibrator')
        path.add_module('ECLShowerShape')
        path.add_module('ECLClusterPSD')
        path.add_module('ECLCovarianceMatrix')
        # The module ECLFinalizer must run after EventT0Combiner


def add_ecl_finalizer_module(path, components=None):
    """
        Add the ECL finalizer module to the path.

        :param path: The path to add the modules to.
        :param components: The components to use or None to use all standard components.
        """

    if components is None or 'ECL' in components:
        path.add_module('ECLFinalizer')


def add_ecl_track_cluster_modules(path, components=None):
    """
    Add the ECL track cluster matching module to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or ('ECL' in components and ('PXD' in components or 'SVD' in components or 'CDC' in components)):
        path.add_module('ECLTrackClusterMatching')


def add_ecl_cluster_properties_modules(path, components=None):
    """
    Add the ECL cluster properties module to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or ('ECL' in components and ('PXD' in components or 'SVD' in components or 'CDC' in components)):
        path.add_module('ECLClusterProperties')


def add_ecl_track_brem_finder(path, components=None):
    """
    Add the bremsstrahlung finding module to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or ('ECL' in components and ('PXD' in components or 'SVD' in components)):
        path.add_module('ECLTrackBremFinder')


def add_ecl_chargedpid_module(path, components=None):
    """
    Add the ECL charged PID module to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'ECL' in components:
        path.add_module('ECLChargedPID')


def add_ecl_mc_matcher_module(path, components=None):
    """
    Add the ECL MC matcher module to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'ECL' in components:
        path.add_module('MCMatcherECLClusters')


def add_ext_module(path, components=None):
    """
    Add the extrapolation module to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    if components is None or 'CDC' in components:
        path.add_module('Ext')


def add_dedx_modules(path, components=None):
    """
    Add the dE/dX reconstruction modules to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    # CDC dE/dx PID
    if components is None or 'CDC' in components:
        path.add_module('CDCDedxPID')
    # VXD dE/dx PID
    # only run this if the SVD is enabled - PXD is disabled by default
    if components is None or 'SVD' in components:
        path.add_module('VXDDedxPID')


def prepare_cdst_analysis(path, components=None, mc=False, add_eventt0_combiner=False):
    """
    Adds to a (analysis) path all the modules needed to analyse a cDST file in the raw+tracking format
    for collisions/cosmics data or in the digits+tracking format for MC data.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    :param mc: Are we running over MC data or not? If so, do not run the unpackers.
    :param add_eventt0_combiner: If True, it adds the EventT0Combiner module when the post-tracking
      reconstruction is run. This must NOT be used during the calibration, but it may be necessary
      for validation purposes or for the user analyses.
    """
    # Add the unpackers only if not running on MC, otherwise check the components and simply add
    # the Gearbox and the Geometry modules
    if not mc:
        add_unpackers(path,
                      components=components)
    else:
        check_components(components)
        path.add_module('Gearbox')
        path.add_module('Geometry')

    # This currently just calls add_ecl_modules
    add_prefilter_pretracking_reconstruction(path,
                                             components=components)

    # Needed to retrieve the PXD and SVD clusters out of the raw data
    if components is None or 'SVD' in components:
        add_svd_reconstruction(path)
    if components is None or 'PXD' in components:
        add_pxd_reconstruction(path)

    # check, this one may not be needed...
    path.add_module('SetupGenfitExtrapolation',
                    energyLossBrems=False,
                    noiseBrems=False)

    # Add the posttracking modules needed for the cDST analysis
    add_prefilter_posttracking_reconstruction(path,
                                              components=components,
                                              for_cdst_analysis=True,
                                              add_eventt0_combiner_for_cdst=add_eventt0_combiner)


def prepare_user_cdst_analysis(path, components=None, mc=False):
    """
    Adds to a (analysis) path all the modules needed to analyse a cDST file in the raw+tracking format
    for collisions/cosmics data or in the digits+tracking format for MC data.
    Differently from prepare_cdst_analysis(), this function add the EventT0Combiner module to the path,
    which makes this function suitable for all the users and not only for the calibration expertes.
    Note that the EventT0Combiner module is necessary for applying the proper EventT0 correction to
    our data.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    :param mc: Are we running over MC data or not? If so, do not run the unpackers.
    """
    prepare_cdst_analysis(path=path, components=components, mc=mc, add_eventt0_combiner=True)
