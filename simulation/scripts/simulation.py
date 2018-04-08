#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
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
                B2ERROR("Duplicate module in path: %s" % module_type)
            else:
                found.append(module.type())

    if len(required) != len(found):
        # Apparently at least one module is missing
        for r in required:
            if r not in found:
                B2ERROR("No '%s' module found but needed for simulation" % r)
    # We have all modules but do they have the correct order?
    elif required != found:
        B2ERROR("Simulation modules in wrong order. Should be '%s' but is '%s'"
                % (", ".join(required), ", ".join(found)))


def add_PXDDataReduction(path, components,
                         pxd_unfiltered_digits='pxd_unfiltered_digits',
                         doCleanup=True):

    # SVD reconstruction
    svd_cluster = '__ROIsvdClusters'
    add_svd_reconstruction(path, isROIsimulation=True)

    # SVD tracking
    svd_reco_tracks = '__ROIsvdRecoTracks'

    add_tracking_for_PXDDataReduction_simulation(path, components, svd_cluster='__ROIsvdClusters')

    add_roiFinder(path, svd_reco_tracks)

    # Filtering of PXDDigits
    pxd_digifilter = register_module('PXDdigiFilter')
    pxd_digifilter.param('ROIidsName', 'ROIs')
    pxd_digifilter.param('PXDDigitsName', pxd_unfiltered_digits)
    pxd_digifilter.param('PXDDigitsInsideROIName', 'PXDDigits')
    path.add_module(pxd_digifilter)

    # empty the StoreArrays which were used for the PXDDatareduction as those are not needed anymore
    if doCleanup:
        datastore_cleaner = register_module('PruneDataStore')
        datastore_cleaner.param('keepMatchedEntries', False)
        datastore_cleaner.param('matchEntries', ['ROIs', '__ROIsvdRecoDigits', '__ROIsvdClusters', '__ROIsvdRecoTracks',
                                                 'SPTrackCands__ROI', 'SpacePoints__ROI', pxd_unfiltered_digits,
                                                 # till here it are StoreArrays, the following are relations and Datastore objects
                                                 'SegmentNetwork__ROI', 'PXDInterceptsToROIs',
                                                 'RecoHitInformationsTo__ROIsvdClusters',
                                                 'SpacePoints__ROITo__ROIsvdClusters', '__ROIsvdClustersToMCParticles',
                                                 '__ROIsvdClustersToSVDDigits', '__ROIsvdClustersToSVDTrueHits',
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

    pxdDataRed = register_module('PXDROIFinder')
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
        usePXDDataReduction=True,
        cleanupPXDDataReduction=True,
        generate_2nd_cdc_hits=False,
        simulateT0jitter=False,
        usePXDGatedMode=False):
    """
    This function adds the standard simulation modules to a path.
    @param cleanupPXDDataReduction: if True the datastore objects used by PXDDataReduction are emptied
    """

    # background mixing or overlay input before process forking
    if bkgfiles:
        if bkgOverlay:
            bkginput = register_module('BGOverlayInput')
            bkginput.param('inputFileNames', bkgfiles)
            path.add_module(bkginput)
        else:
            bkgmixer = register_module('BeamBkgMixer')
            bkgmixer.param('backgroundFiles', bkgfiles)
            if components:
                bkgmixer.param('components', components)
            path.add_module(bkgmixer)
            if usePXDGatedMode:
                if components is None or 'PXD' in components:
                    # Emulate injection vetos for PXD
                    pxd_veto_emulator = register_module('PXDInjectionVetoEmulator')
                    path.add_module(pxd_veto_emulator)

    # geometry parameter database
    if 'Gearbox' not in path:
        gearbox = register_module('Gearbox')
        path.add_module(gearbox)

    # detector geometry
    if 'Geometry' not in path:
        geometry = register_module('Geometry', useDB=True)
        if components is not None:
            B2WARNING("Custom detector components specified, disabling Geometry from Database")
            geometry.param('useDB', False)
            geometry.param('components', components)
        path.add_module(geometry)

    # event T0 jitter simulation
    if simulateT0jitter and 'EventT0Generator' not in path:
        eventt0 = register_module('EventT0Generator')
        path.add_module(eventt0)

    # detector simulation
    if 'FullSim' not in path:
        g4sim = register_module('FullSim')
        path.add_module(g4sim)

    check_simulation(path)

    # no checks are performed for BeamBkgMixer and the Digitizers as they are
    # not necessary for running simulation jobs and it should be possible to
    # have them in the path more than once

    # SVD digitization
    if components is None or 'SVD' in components:
        add_svd_simulation(path)

    # CDC digitization
    if components is None or 'CDC' in components:
        cdc_digitizer = register_module('CDCDigitizer')
        cdc_digitizer.param("Output2ndHit", generate_2nd_cdc_hits)
        path.add_module(cdc_digitizer)

    # PXD digitization
    pxd_digits_name = ''
    if components is None or 'PXD' in components:
        if usePXDDataReduction:
            pxd_digits_name = 'pxd_unfiltered_digits'
        add_pxd_simulation(path, digitsName=pxd_digits_name)

    # TOP digitization
    if components is None or 'TOP' in components:
        top_digitizer = register_module('TOPDigitizer')
        path.add_module(top_digitizer)

    # ARICH digitization
    if components is None or 'ARICH' in components:
        arich_digitizer = register_module('ARICHDigitizer')
        path.add_module(arich_digitizer)

    # ECL digitization
    if components is None or 'ECL' in components:
        ecl_digitizer = register_module('ECLDigitizer')
        if bkgfiles:
            ecl_digitizer.param('Background', 1)
        path.add_module(ecl_digitizer)

    # BKLM digitization
    if components is None or 'BKLM' in components:
        bklm_digitizer = register_module('BKLMDigitizer')
        path.add_module(bklm_digitizer)

    # EKLM digitization
    if components is None or 'EKLM' in components:
        eklm_digitizer = register_module('EKLMDigitizer')
        path.add_module(eklm_digitizer)

    # background overlay executor - after all digitizers
    if bkgfiles and bkgOverlay:
        path.add_module('BGOverlayExecutor', PXDDigitsName=pxd_digits_name)
        if components is None or 'PXD' in components:
            path.add_module("PXDDigitSorter", digits=pxd_digits_name)
        # sort SVDShaperDigits before PXD data reduction
        if components is None or 'SVD' in components:
            path.add_module("SVDShaperDigitSorter")

    # PXD data reduction - after background overlay executor
    if (components is None or 'PXD' in components) and usePXDDataReduction:
        add_PXDDataReduction(path, components, pxd_digits_name, doCleanup=cleanupPXDDataReduction)

    # statistics summary
    path.add_module('StatisticsSummary').set_name('Sum_Simulation')
