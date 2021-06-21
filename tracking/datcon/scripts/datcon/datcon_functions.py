#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
from svd import add_svd_reconstruction


def add_datcon(path,
               svd_space_points='SVDSpacePoints',
               svd_clusters='SVDClusters',
               datcon_reco_tracks='DATCONRecoTracks',
               use_simple_roi_calculation=True,
               use_advanced_roi_calculation=False,
               datcon_pxd_intercepts_name='DATCONPXDIntercepts',
               datcon_rois_name='DATCONROIs'):
    """
    Convenience function to add the optimized DATCON to the path.
    : param path: The path to add the DATCON module to.
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
                    SVDSpacePointStoreArrayName=svd_space_points,
                    SVDClustersStoreArrayName=svd_clusters,
                    finalOverlapResolverNameSVDClusters=svd_clusters,
                    refinerOverlapResolverNameSVDClusters=svd_clusters,

                    minimumUClusterTime=-25,
                    minimumVClusterTime=-25,
                    maximumUClusterTime=40,
                    maximumVClusterTime=40,

                    simpleVerticalHoughSpaceSize=0.25,
                    simpleNAngleSectors=256,
                    simpleNVerticalSectors=256,
                    simpleMinimumHSClusterSize=2,
                    simpleMaximumHSClusterSize=10,

                    relationFilter='angleAndTime',
                    relationFilterParameters={'AngleAndTimeThetaCutDeltaL0': 0.03,
                                              'AngleAndTimeThetaCutDeltaL1': 0.10,
                                              'AngleAndTimeThetaCutDeltaL2': 0.20,
                                              'AngleAndTimeDeltaUTime': 15.,
                                              'AngleAndTimeDeltaVTime': 15., },

                    maxRelations=2000,
                    twoHitFilterLimit=50,
                    twoHitFilter='twoHitVirtualIPQI',
                    twoHitUseNBestHits=5,
                    threeHitUseNBestHits=5,
                    fourHitUseNBestHits=3,
                    fiveHitUseNBestHits=2,

                    minQualitiyIndicatorSize3=0.001,
                    minQualitiyIndicatorSize4=0.001,
                    minQualitiyIndicatorSize5=0.001,
                    maxNumberOfEachPathLength=10,

                    RecoTracksStoreArrayName=datcon_reco_tracks,

                    calculateROI=use_simple_roi_calculation,
                    # tolerancePhi=0,
                    # toleranceZ=0,
                    multiplierUL1=600,
                    multiplierUL2=800,
                    multiplierVL1=0.8,
                    multiplierVL2=1.2,
                    radiusCorrectionFactor=4.0,
                    sinPhiCorrectionFactor=0.0,
                    cosPhiCorrectionFactor=0.0,
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


def add_fpga_datcon(path):
    """
    Convenience function to add the FPGA version of DATCON to the path.
    : param path: The path to add the DATCONFPGA module to.
    """

    path.add_module('DATCONFPGA',
                    uSideVerticalHoughSpaceSize=100000000,
                    uSideNAngleSectors=128,
                    uSideNVerticalSectors=128,
                    uSideMinimumHSClusterSize=5,
                    uSideMaximumHSClusterSize=50,
                    vSideIsUClusterizer=False,    # required, as default is True for u side
                    vSideIsUFinder=False,         # required, as default is True for u side
                    vSideNAngleSectors=128,
                    vSideNVerticalSectors=64,
                    vSideMinimumHSClusterSize=5,
                    vSideMaximumHSClusterSize=50,
                    vSideMinimumX=-75. / 180. * 3.14159265359,  # corresponds to  +15° for theta (default values are for u side)
                    vSideMaximumX=65. / 180. * 3.14159265359,  # corresponds to +155° for theta (default values are for u side)
                    vSideVerticalHoughSpaceSize=20000000,
                    createPXDIntercepts=False,
                    extrapolationPhiCutL1=1.,
                    extrapolationPhiCutL2=1.,
                    uROIsizeL1=80,
                    uROIsizeL2=60,
                    vROIsizeL1=80,
                    vROIsizeL2=60)
