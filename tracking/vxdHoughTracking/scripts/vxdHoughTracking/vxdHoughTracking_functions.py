#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2


def add_VXDHoughTracking(path,
                         svd_space_points='SVDSpacePoints',
                         svd_clusters='SVDClusters',
                         reco_tracks='VXDHoughTrackingRecoTracks',
                         use_simple_roi_calculation=False,
                         usePXDROIFinderModule=True,
                         pxd_intercepts_name='VXDHoughTrackingPXDIntercepts',
                         rois_name='VXDHoughTrackingROIs'):
    """
    Convenience function to add the optimized VXDHoughTracking to the path.
    : param path: The path to add the VXDHoughTracking module to.
    : param reco_tracks: Name of the StoreArray containing the RecoTracks found by VXDHoughTracking
    : param use_simple_roi_calculation: Use a simple ROI calculation with a circle extrapolation in r-phi
        and a straight line extrapolation in theta
    : param usePXDROIFinderModule: Calculate ROI using the PXDROIFinderModule
    : param pxd_intercepts_name: Name of the StoreArray containing the PXDIntercepts calculated by VXDHoughTracking.
        If both simple and advanced ROI finding are selected, the StoreArray for the advanced ROI finding will get
        the prefix PXDROIFinder,
    : param rois_name: Name of the StoreArray containing the ROIs calculated by VXDHoughTracking.
        If both simple and advanced ROI finding are selected, the StoreArray for the advanced ROI finding will get
        the prefix PXDROIFinder,
    """

    advancedPXDInterceptsName = pxd_intercepts_name
    advancedROIName = rois_name
    # If both simple VXDHoughTracking ROI finding and regular ROI finding using the PXDROIFinder with the RecoTracks found by
    # VXDHoughTracking shall be performed, set a different name for the PXDInterecepts and the ROIs from the PXDROIFinder
    # (advancedPXDInterceptsName and advancedROINames) to be able to distinguish them.
    if use_simple_roi_calculation and usePXDROIFinderModule:
        advancedPXDInterceptsName = 'PXDROIFinder' + advancedPXDInterceptsName
        advancedROIName = 'PXDROIFinder' + advancedROIName

    path.add_module('VXDHoughTracking',
                    SVDSpacePointStoreArrayName=svd_space_points,
                    SVDClustersStoreArrayName=svd_clusters,
                    finalOverlapResolverNameSVDClusters=svd_clusters,
                    refinerOverlapResolverNameSVDClusters=svd_clusters,

                    relationFilter='angleAndTime',
                    relationFilterParameters={'AngleAndTimeThetaCutDeltaL0': 0.03,
                                              'AngleAndTimeThetaCutDeltaL1': 0.10,
                                              'AngleAndTimeThetaCutDeltaL2': 0.20,
                                              'AngleAndTimeDeltaUTime': 15.,
                                              'AngleAndTimeDeltaVTime': 15., },

                    twoHitUseNBestHits=5,
                    threeHitUseNBestHits=5,
                    fourHitUseNBestHits=3,
                    fiveHitUseNBestHits=2,
                    )

    if usePXDROIFinderModule:

        if 'SetupGenfitExtrapolation' not in path:
            path.add_module('SetupGenfitExtrapolation').set_name('SetupGenfitExtrapolationForVXDHoughTracking')

        # TRACK FITTING
        path.add_module('DAFRecoFitter', recoTracksStoreArrayName=reco_tracks).set_name('VXDHoughTracking-only DAFRecoFitter')

        path.add_module('PXDROIFinder',
                        recoTrackListName=reco_tracks,
                        PXDInterceptListName=advancedPXDInterceptsName,
                        ROIListName=advancedROIName,
                        tolerancePhi=0.15,
                        toleranceZ=0.5,
                        sigmaSystU=0.02,
                        sigmaSystV=0.02,
                        numSigmaTotU=10,
                        numSigmaTotV=10,
                        maxWidthU=0.5,
                        maxWidthV=0.5,).set_name('VXDHoughTrackingPXDROIFinder')
