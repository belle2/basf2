#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from geometry import check_components
from L1trigger import add_trigger_simulation
from pxd import add_pxd_simulation
from svd import add_svd_simulation
from svd import add_svd_reconstruction
from tracking import add_tracking_for_PXDDataReduction_simulation, add_roiFinder


def check_simulation(path):
    """Check if the minimum number of modules required for simulation are in
    the path and in the correct order"""
    required = ['Gearbox', 'Geometry', 'FullSim']
    found = []

    # make a list of all required modules in the path
    for module in path.modules():
        module_type = module.type()
        # if required add to list of found modules
        if module_type in required:
            # unless it is already in there
            if module_type in found:
                b2.B2ERROR(f"Duplicate module in path: {module_type}")
            else:
                found.append(module.type())

    if len(required) != len(found):
        # Apparently at least one module is missing
        for r in required:
            if r not in found:
                b2.B2ERROR(f"No '{r}' module found but needed for simulation")
    # We have all modules but do they have the correct order?
    elif required != found:
        b2.B2ERROR(f"Simulation modules in wrong order. Should be '{', '.join(required)}' but is '{', '.join(found)}'")


def add_PXDDataReduction(path, components, pxd_unfiltered_digits='pxd_unfiltered_digits',
                         doCleanup=True, overrideDB=False, usePXDDataReduction=True, save_slow_pions_in_mc=False):
    """
    This function adds the standard simulation modules to a path.
    @param pxd_unfiltered_digits: the name of the StoreArray containing the input PXDDigits
    @param overrideDB: override settings from the DB with the value set in 'usePXDDataReduction'
    @param usePXDDataReduction: if 'overrideDB==True', override settings from the DB
    @param doCleanup: if 'doCleanup=True' temporary datastore objects are emptied
    @param save_slow_pions_in_mc: if True, additional Regions of Interest on the PXD are created to save the PXDDigits
      of slow pions from D* -> D pi^{\\pm} decays using the MCSlowPionPXDROICreator based on MC truth information
    """

    # SVD reconstruction
    # svd_cluster = '__ROIsvdClusters'
    add_svd_reconstruction(path, isROIsimulation=True)

    # SVD tracking
    svd_reco_tracks = '__ROIsvdRecoTracks'

    add_tracking_for_PXDDataReduction_simulation(path, components, svd_cluster='__ROIsvdClusters')

    add_roiFinder(path, svd_reco_tracks)

    if save_slow_pions_in_mc:
        path.add_module('MCSlowPionPXDROICreator', PXDDigitsName=pxd_unfiltered_digits, ROIsName='ROIs')

    # Filtering of PXDDigits
    pxd_digifilter = b2.register_module('PXDdigiFilter')
    pxd_digifilter.param('ROIidsName', 'ROIs')
    pxd_digifilter.param('PXDDigitsName', pxd_unfiltered_digits)
    pxd_digifilter.param('PXDDigitsInsideROIName', 'PXDDigits')
    pxd_digifilter.param('overrideDB', overrideDB)
    pxd_digifilter.param('usePXDDataReduction', usePXDDataReduction)  # only used for overrideDB=True
    path.add_module(pxd_digifilter)

    # empty the StoreArrays which were used for the PXDDatareduction as those are not needed anymore
    if doCleanup:
        datastore_cleaner = b2.register_module('PruneDataStore')
        datastore_cleaner.param('keepMatchedEntries', False)
        datastore_cleaner.param('matchEntries', ['ROIs', '__ROIsvdRecoDigits', '__ROIsvdClusters', '__ROIsvdRecoTracks',
                                                 'SPTrackCands__ROI', 'SpacePoints__ROI', pxd_unfiltered_digits,
                                                 # till here it are StoreArrays, the following are relations and Datastore objects
                                                 'SegmentNetwork__ROI', 'PXDInterceptsToROIs',
                                                 'RecoHitInformationsTo__ROIsvdClusters',
                                                 'SpacePoints__ROITo__ROIsvdClusters', '__ROIsvdClustersToMCParticles',
                                                 '__ROIsvdRecoDigitsToMCParticles',
                                                 '__ROIsvdClustersTo__ROIsvdRecoDigits', '__ROIsvdClustersToSVDTrueHits',
                                                 '__ROIsvdClustersTo__ROIsvdRecoTracks', '__ROIsvdRecoTracksToPXDIntercepts',
                                                 '__ROIsvdRecoTracksToRecoHitInformations',
                                                 '__ROIsvdRecoTracksToSPTrackCands__ROI',
                                                 # not only prune the pxd_unfiltered_digits, but also their relations to
                                                 # MCParticles, PXDDigits (the filtered ones), and PXDTrueHits
                                                 f'{pxd_unfiltered_digits}ToMCParticles',
                                                 f'{pxd_unfiltered_digits}ToPXDDigits',
                                                 f'{pxd_unfiltered_digits}ToPXDTrueHits'])
        path.add_module(datastore_cleaner)


def add_simulation(
        path,
        components=None,
        bkgfiles=None,
        bkgOverlay=True,
        forceSetPXDDataReduction=False,
        usePXDDataReduction=True,
        cleanupPXDDataReduction=True,
        generate_2nd_cdc_hits=False,
        simulateT0jitter=True,
        isCosmics=False,
        FilterEvents=False,
        usePXDGatedMode=False,
        skipExperimentCheckForBG=False,
        save_slow_pions_in_mc=False):
    """
    This function adds the standard simulation modules to a path.
    @param forceSetPXDDataReduction: override settings from the DB with the value set in 'usePXDDataReduction'
    @param usePXDDataReduction: if 'forceSetPXDDataReduction==True', override settings from the DB
    @param cleanupPXDDataReduction: if True the datastore objects used by PXDDataReduction are emptied
    @param simulateT0jitter: if True simulate L1 trigger jitter
    @param isCosmics: if True the filling pattern is removed from L1 jitter simulation
    @param FilterEvents: if True only the events that pass the L1 trigger will survive simulation, the other are discarded.
        Make sure you do need to filter events before you set the value to True.
    @param skipExperimentCheckForBG: If True, skip the check on the experiment number consistency between the basf2
      process and the beam background files. Note that this check should be skipped only by experts.
    @param save_slow_pions_in_mc: if True, additional Regions of Interest on the PXD are created to save the PXDDigits
      of slow pions from D* -> D pi^{\\pm} decays using the MCSlowPionPXDROICreator based on MC truth information
    """

    path.add_module('StatisticsSummary').set_name('Sum_PreSimulation')

    # Check compoments.
    check_components(components)

    # background mixing or overlay input before process forking
    if bkgfiles is not None:
        if bkgOverlay:
            bkginput = b2.register_module('BGOverlayInput')
            bkginput.param('inputFileNames', bkgfiles)
            bkginput.param('skipExperimentCheck', skipExperimentCheckForBG)
            path.add_module(bkginput)
        else:
            bkgmixer = b2.register_module('BeamBkgMixer')
            bkgmixer.param('backgroundFiles', bkgfiles)
            if components:
                bkgmixer.param('components', components)
            path.add_module(bkgmixer)
            if usePXDGatedMode:
                if components is None or 'PXD' in components:
                    # PXD is sensitive to hits in intervall -20us to +20us
                    bkgmixer.param('minTimePXD', -20000.0)
                    bkgmixer.param('maxTimePXD', 20000.0)
                    # Emulate injection vetos for PXD
                    pxd_veto_emulator = b2.register_module('PXDInjectionVetoEmulator')
                    path.add_module(pxd_veto_emulator)

    # geometry parameter database
    if 'Gearbox' not in path:
        gearbox = b2.register_module('Gearbox')
        path.add_module(gearbox)

    # detector geometry
    if 'Geometry' not in path:
        path.add_module('Geometry', useDB=True)
        if components is not None:
            b2.B2WARNING("Custom detector components specified: Will still build full geometry")

    # event T0 jitter simulation
    if simulateT0jitter and 'EventT0Generator' not in path:
        eventt0 = b2.register_module('EventT0Generator')
        eventt0.param("isCosmics", isCosmics)
        path.add_module(eventt0)

    # create EventLevelTriggerTimeInfo if it doesn't exist in BG Overlay
    if 'SimulateEventLevelTriggerTimeInfo' not in path:
        eventleveltriggertimeinfo = b2.register_module('SimulateEventLevelTriggerTimeInfo')
        path.add_module(eventleveltriggertimeinfo)

    # detector simulation
    if 'FullSim' not in path:
        g4sim = b2.register_module('FullSim')
        path.add_module(g4sim)

    check_simulation(path)

    # no checks are performed for BeamBkgMixer and the Digitizers as they are
    # not necessary for running simulation jobs and it should be possible to
    # have them in the path more than once

    # CDC digitization
    if components is None or 'CDC' in components:
        cdc_digitizer = b2.register_module('CDCDigitizer')
        cdc_digitizer.param("Output2ndHit", generate_2nd_cdc_hits)
        path.add_module(cdc_digitizer)

    # TOP digitization
    if components is None or 'TOP' in components:
        top_digitizer = b2.register_module('TOPDigitizer')
        path.add_module(top_digitizer)

    # ARICH digitization
    if components is None or 'ARICH' in components:
        arich_digitizer = b2.register_module('ARICHDigitizer')
        path.add_module(arich_digitizer)

    # ECL digitization
    if components is None or 'ECL' in components:
        ecl_digitizer = b2.register_module('ECLDigitizer')
        if bkgfiles is not None:
            ecl_digitizer.param('Background', 1)
        path.add_module(ecl_digitizer)

    # KLM digitization
    if components is None or 'KLM' in components:
        klm_digitizer = b2.register_module('KLMDigitizer')
        path.add_module(klm_digitizer)

    # BG Overlay for CDC, TOP, ARICH and KLM (for ECL it's done in ECLDigitizer)
    if bkgfiles is not None and bkgOverlay:
        m = path.add_module('BGOverlayExecutor', components=['CDC', 'TOP', 'ARICH', 'KLM'])
        m.set_name('BGOverlayExecutor_CDC...KLM')

    if components is None or 'TRG' in components:
        add_trigger_simulation(path, simulateT0jitter=simulateT0jitter, FilterEvents=FilterEvents)

    # SVD digitization, BG Overlay, sorting and zero suppression
    if components is None or 'SVD' in components:
        add_svd_simulation(path)
        if bkgfiles is not None and bkgOverlay:
            m = path.add_module('BGOverlayExecutor', components=['SVD'])
            m.set_name('BGOverlayExecutor_SVD')
        path.add_module('SVDShaperDigitSorter')
        path.add_module('SVDZeroSuppressionEmulator')

    # PXD digitization, BG overlay, sorting and data reduction
    if components is None or 'PXD' in components:
        if forceSetPXDDataReduction:
            pxd_digits_name = ''
            if usePXDDataReduction:
                pxd_digits_name = 'pxd_unfiltered_digits'
            add_pxd_simulation(path, digitsName=pxd_digits_name)
            if bkgfiles is not None and bkgOverlay:
                m = path.add_module('BGOverlayExecutor', components=['PXD'], PXDDigitsName=pxd_digits_name)
                m.set_name('BGOverlayExecutor_PXD')
            path.add_module('PXDDigitSorter', digits=pxd_digits_name)
            if usePXDDataReduction:
                add_PXDDataReduction(
                    path,
                    components,
                    pxd_digits_name,
                    doCleanup=cleanupPXDDataReduction,
                    overrideDB=forceSetPXDDataReduction,
                    usePXDDataReduction=usePXDDataReduction,
                    save_slow_pions_in_mc=save_slow_pions_in_mc)
        else:
            # use DB conditional module to decide whether ROI finding should be activated
            path_disableROI_Sim = b2.create_path()
            add_pxd_simulation(path_disableROI_Sim, digitsName='PXDDigits')
            if bkgfiles is not None and bkgOverlay:
                m = path_disableROI_Sim.add_module('BGOverlayExecutor', components=['PXD'], PXDDigitsName='PXDDigits')
                m.set_name('BGOverlayExecutor_PXD')
            path_disableROI_Sim.add_module('PXDDigitSorter', digits='PXDDigits')

            path_enableROI_Sim = b2.create_path()
            add_pxd_simulation(path_enableROI_Sim, digitsName='pxd_unfiltered_digits')
            if bkgfiles is not None and bkgOverlay:
                m = path_enableROI_Sim.add_module('BGOverlayExecutor', components=['PXD'], PXDDigitsName='pxd_unfiltered_digits')
                m.set_name('BGOverlayExecutor_PXD')
            path_enableROI_Sim.add_module('PXDDigitSorter', digits='pxd_unfiltered_digits')
            add_PXDDataReduction(
                path_enableROI_Sim,
                components,
                pxd_unfiltered_digits='pxd_unfiltered_digits',
                doCleanup=cleanupPXDDataReduction,
                save_slow_pions_in_mc=save_slow_pions_in_mc)

            roi_condition_module_Sim = path.add_module('ROIfindingConditionFromDB')
            roi_condition_module_Sim.if_true(path_enableROI_Sim, b2.AfterConditionPath.CONTINUE)
            roi_condition_module_Sim.if_false(path_disableROI_Sim, b2.AfterConditionPath.CONTINUE)

    # statistics summary
    path.add_module('StatisticsSummary').set_name('Sum_Simulation')
