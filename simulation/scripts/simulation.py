#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
from geometry import check_components
from L1trigger import add_tsim
from pxd import add_pxd_simulation
from svd import add_svd_simulation
from svd import add_svd_reconstruction
from tracking import add_tracking_for_PXDDataReduction_simulation


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
                b2.B2ERROR("Duplicate module in path: %s" % module_type)
            else:
                found.append(module.type())

    if len(required) != len(found):
        # Apparently at least one module is missing
        for r in required:
            if r not in found:
                b2.B2ERROR("No '%s' module found but needed for simulation" % r)
    # We have all modules but do they have the correct order?
    elif required != found:
        b2.B2ERROR("Simulation modules in wrong order. Should be '%s' but is '%s'"
                   % (", ".join(required), ", ".join(found)))


def add_PXDDataReduction(path, components, pxd_unfiltered_digits='pxd_unfiltered_digits',
                         doCleanup=True, overrideDB=False, usePXDDataReduction=True):
    """
    This function adds the standard simulation modules to a path.
    @param pxd_unfiltered_digits: the name of the StoreArray containing the input PXDDigits
    @param overrideDB: override settings from the DB with the value set in 'usePXDDataReduction'
    @param usePXDDataReduction: if 'overrideDB==True', override settings from the DB
    @param doCleanup: if 'doCleanup=True' temporary datastore objects are emptied
    """

    # SVD reconstruction
    # svd_cluster = '__ROIsvdClusters'
    add_svd_reconstruction(path, isROIsimulation=True)

    # SVD tracking
    svd_reco_tracks = '__ROIsvdRecoTracks'

    add_tracking_for_PXDDataReduction_simulation(path, components, svd_cluster='__ROIsvdClusters')

    add_roiFinder(path, svd_reco_tracks)

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
                                                 '__ROIsvdRecoTracksToSPTrackCands__ROI'])
        path.add_module(datastore_cleaner)


def add_roiFinder(path, reco_tracks):
    """
    Add the ROI finding to the path creating ROIs out of reco tracks by extrapolating them to the PXD volume.
    :param path: Where to add the module to.
    :param reco_tracks: Which tracks to use in the extrapolation step.
    """

    pxdDataRed = b2.register_module('PXDROIFinder')
    param_pxdDataRed = {
        'recoTrackListName': reco_tracks,
        'PXDInterceptListName': 'PXDIntercepts',
        'ROIListName': 'ROIs',
        'tolerancePhi': 0.15,
        'toleranceZ': 0.5,
        'sigmaSystU': 0.02,
        'sigmaSystV': 0.02,
        'numSigmaTotU': 10,
        'numSigmaTotV': 10,
        'maxWidthU': 0.5,
        'maxWidthV': 0.5,
    }
    pxdDataRed.param(param_pxdDataRed)
    path.add_module(pxdDataRed)


def add_simulation(
        path,
        components=None,
        bkgfiles=None,
        bkgOverlay=True,
        forceSetPXDDataReduction=False,
        usePXDDataReduction=True,
        cleanupPXDDataReduction=True,
        generate_2nd_cdc_hits=False,
        simulateT0jitter=False,
        usePXDGatedMode=False,
        skipExperimentCheckForBG=False):
    """
    This function adds the standard simulation modules to a path.
    @param forceSetPXDDataReduction: override settings from the DB with the value set in 'usePXDDataReduction'
    @param usePXDDataReduction: if 'forceSetPXDDataReduction==True', override settings from the DB
    @param cleanupPXDDataReduction: if True the datastore objects used by PXDDataReduction are emptied
    @param simulateT0jitter: if True simulate L1 trigger jitter
    @param skipExperimentCheckForBG: If True, skip the check on the experiment number consistency between the basf2
      process and the beam background files. Note that this check should be skipped only by experts.
    """

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
        path.add_module(eventt0)

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

    # TO DO:
    # add BG Overlay for: KLM, ECL, ARICH, TOP, CDC

    # TO DO: L1 TRIGGER simulation
    # !!! DO NOT UNCOMMENT !!!
    # if simulateL1trigger:
    #    add_tsim(path)

    # SVD digitization
    if components is None or 'SVD' in components:
        add_svd_simulation(path)

    # PXD digitization
    pxd_digits_name = ''
    if components is None or 'PXD' in components:
        if forceSetPXDDataReduction:
            if usePXDDataReduction:
                pxd_digits_name = 'pxd_unfiltered_digits'
            add_pxd_simulation(path, digitsName=pxd_digits_name)
        else:
            # use DB conditional module to decide whether ROI finding should be activated
            path_disableROI_Sim = b2.create_path()
            path_enableROI_Sim = b2.create_path()

            add_pxd_simulation(path_disableROI_Sim, digitsName='PXDDigits')
            add_pxd_simulation(path_enableROI_Sim, digitsName='pxd_unfiltered_digits')

            roi_condition_module_Sim = path.add_module("ROIfindingConditionFromDB")
            roi_condition_module_Sim.if_true(path_enableROI_Sim, b2.AfterConditionPath.CONTINUE)
            roi_condition_module_Sim.if_false(path_disableROI_Sim, b2.AfterConditionPath.CONTINUE)

    # background overlay executor - after all digitizers
    # TO DO: overlay ONLY for PXD and SVD
    if bkgfiles is not None and bkgOverlay:
        if forceSetPXDDataReduction:
            path.add_module('BGOverlayExecutor', PXDDigitsName=pxd_digits_name)

            if components is None or 'PXD' in components:
                path.add_module("PXDDigitSorter", digits=pxd_digits_name)

            # sort SVDShaperDigits before PXD data reduction
            if components is None or 'SVD' in components:
                path.add_module("SVDShaperDigitSorter")
        else:
            path_disableROI_Bkg = b2.create_path()
            path_enableROI_Bkg = b2.create_path()

            path_disableROI_Bkg.add_module('BGOverlayExecutor', PXDDigitsName='PXDDigits')
            if components is None or 'PXD' in components:
                path_disableROI_Bkg.add_module("PXDDigitSorter", digits='PXDDigits')
            if components is None or 'SVD' in components:
                path_disableROI_Bkg.add_module("SVDShaperDigitSorter")

            path_enableROI_Bkg.add_module('BGOverlayExecutor', PXDDigitsName='pxd_unfiltered_digits')
            if components is None or 'PXD' in components:
                path_enableROI_Bkg.add_module("PXDDigitSorter", digits='pxd_unfiltered_digits')
            if components is None or 'SVD' in components:
                path_enableROI_Bkg.add_module("SVDShaperDigitSorter")

            roi_condition_module_Bkg = path.add_module("ROIfindingConditionFromDB")
            roi_condition_module_Bkg.if_true(path_enableROI_Bkg, b2.AfterConditionPath.CONTINUE)
            roi_condition_module_Bkg.if_false(path_disableROI_Bkg, b2.AfterConditionPath.CONTINUE)

    if components is None or 'SVD' in components:
        path.add_module("SVDZeroSuppressionEmulator")

    # PXD data reduction - after background overlay executor
    if components is None or 'PXD' in components:
        if forceSetPXDDataReduction:
            if usePXDDataReduction:
                add_PXDDataReduction(path, components, pxd_digits_name, doCleanup=cleanupPXDDataReduction,
                                     overrideDB=forceSetPXDDataReduction, usePXDDataReduction=usePXDDataReduction)
        else:
            path_enableROI_Red = b2.create_path()
            add_PXDDataReduction(
                path_enableROI_Red,
                components,
                pxd_unfiltered_digits='pxd_unfiltered_digits',
                doCleanup=cleanupPXDDataReduction)

            roi_condition_module_Red = path.add_module("ROIfindingConditionFromDB")
            roi_condition_module_Red.if_true(path_enableROI_Red, b2.AfterConditionPath.CONTINUE)

    # statistics summary
    path.add_module('StatisticsSummary').set_name('Sum_Simulation')
