#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
from svd import add_svd_reconstruction


def add_datcon(path,
               rootFileName='trackCandAnalysis.root',
               datcon_reco_tracks='DATCONRecoTracks',
               use_simple_roi_calculation=False,
               use_advanced_roi_calculation=False,
               datcon_pxd_intercepts_name='DATCONPXDIntercepts',
               datcon_rois_name='DATCONROIs'):
    """
    Convenience function to add the optimized DATCON to the path.
    : param path: The path to add the DATCON module to.
    : param rootFileName: Name of the ROOT file for debugging. Will be removed when the full development is finished.
    : param datcon_reco_tracks: Name of the StoreArray containing the RecoTracks found by DATCON
    : param use_simple_roi_calculation: Use a simple ROI calculation with a circle extrapolation in r-phi
        and a straight line extrapolation in theta
    : param use_advanced_roi_calculation: Calculate ROI using the PXDROIFinderModule
    : param datcon_pxd_intercepts_name: Name of the StoreArray containing the PXDIntercepts calculated by DATCON.
        If both simple and advanced ROI finding are selected, the StoreArray for the advanced ROI finding will get
        the prefix PXDROIFinder,
    : param datcon_rois_name: Name of the StoreArray containing the ROIs calculated by DATCON.
        If both simple and advanced ROI finding are selected, the StoreArray for the advanced ROI finding will get
        the prefix PXDROIFinder,
    """

    advancedPXDInterceptsName = datcon_pxd_intercepts_name
    advancedROIName = datcon_rois_name
    # If both simple DATCON ROI finding and regular ROI finding using the PXDROIFinder with the RecoTracks found by
    # DATCON shall be performed, set a different name for the PXDInterecepts and the ROIs from the PXDROIFinder
    # (advancedPXDInterceptsName and advancedROINames) to be able to distinguish them.
    if use_simple_roi_calculation and use_advanced_roi_calculation:
        advancedPXDInterceptsName = 'PXDROIFinder' + advancedPXDInterceptsName
        advancedROIName = 'PXDROIFinder' + advancedROIName

    path.add_module('DATCON',
                    minimumUClusterTime=-10,
                    minimumVClusterTime=-10,
                    maximumUClusterTime=60,
                    maximumVClusterTime=60,

                    simpleVerticalHoughSpaceSize=0.25,
                    simpleNAngleSectors=256,
                    simpleNVerticalSectors=256,
                    simpleMinimumHSClusterSize=4,
                    simpleMaximumHSClusterSize=200,

                    maxRelations=1000000,
                    relationFilter='angleAndTime',
                    relationFilterParameters={'AngleAndTimeThetaCutDeltaL0': 0.05,
                                              'AngleAndTimeThetaCutDeltaL1': 0.10,
                                              'AngleAndTimeThetaCutDeltaL2': 0.25,
                                              'AngleAndTimeDeltaUTime': 50.,
                                              'AngleAndTimeDeltaVTime': 50., },
                    rootFileName=rootFileName,
                    maxRelationsCleaner=1000,
                    # twoHitFilter='twoHitVirtualIPQI',
                    # twoHitUseNBestHits=100,
                    twoHitFilter='all',
                    twoHitUseNBestHits=0,
                    threeHitUseNBestHits=50,
                    fourHitUseNBestHits=30,
                    fiveHitUseNBestHits=10,

                    # trackQualityEstimationMethod='helixFit',
                    minQualitiyIndicatorSize3=0.50,
                    minQualitiyIndicatorSize4=0.001,
                    minQualitiyIndicatorSize5=0.001,
                    maxNumberOfHitsForEachPathLength=50,

                    RecoTracksStoreArrayName=datcon_reco_tracks,
                    )

    if use_advanced_roi_calculation:

        if 'SetupGenfitExtrapolation' not in path:
            path.add_module('SetupGenfitExtrapolation').set_name('SetupGenfitExtrapolationForDATCON')

        # TRACK FITTING
        path.add_module('DAFRecoFitter', recoTracksStoreArrayName=datcon_reco_tracks).set_name('DATCON-only DAFRecoFitter')

        path.add_module('PXDROIFinder',
                        recoTrackListName=datcon_reco_tracks,
                        PXDInterceptListName=advancedPXDInterceptsName,
                        ROIListName=advancedROIName,
                        tolerancePhi=0.15,
                        toleranceZ=0.5,
                        sigmaSystU=0.02,
                        sigmaSystV=0.02,
                        numSigmaTotU=10,
                        numSigmaTotV=10,
                        maxWidthU=0.5,
                        maxWidthV=0.5,).set_name('DATCONPXDROIFinder')
