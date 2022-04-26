#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


def add_svd_hough_tracking(path,
                           svd_space_points='SVDSpacePoints',
                           svd_clusters='SVDClusters',
                           reco_tracks='RecoTracks',
                           svd_space_point_track_candidates='SPTrackCands',
                           suffix=''):
    """
    Convenience function to add the SVDHoughTracking to the path.
    :param path: The path to add the SVDHoughTracking module to.
    :param svd_space_points: Name of the StoreArray containing the SVDSpacePoints
    :param svd_clusters: Name of the StoreArray containing the SVDClusters
    :param reco_tracks: Name of the StoreArray containing the RecoTracks
    :param svd_space_point_track_candidates: Name of the StoreArray containing the SpacePointTrackCandidates
    :param suffix: all names of intermediate StoreArrays will have the suffix appended. Useful in cases someone needs to
                   put several instances of track finding in one path.
    """

    path.add_module('SVDHoughTracking',
                    SVDSpacePointStoreArrayName=svd_space_points + suffix,
                    SVDClustersStoreArrayName=svd_clusters + suffix,
                    finalOverlapResolverNameSVDClusters=svd_clusters + suffix,
                    refinerOverlapResolverNameSVDClusters=svd_clusters + suffix,
                    RecoTracksStoreArrayName=reco_tracks + suffix,
                    SVDSpacePointTrackCandsStoreArrayName=svd_space_point_track_candidates + suffix,

                    relationFilter='angleAndTime',

                    twoHitUseNBestHits=4,
                    threeHitUseNBestHits=3,
                    fourHitUseNBestHits=3,
                    fiveHitUseNBestHits=2,
                    )
