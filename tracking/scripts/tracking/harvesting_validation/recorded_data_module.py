#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


from ROOT import Belle2

import numpy as np

import tracking.harvest.harvesting as harvesting
import tracking.harvest.refiners as refiners
import tracking.harvest.peelers as peelers


class EventInfoHarvester(harvesting.HarvestingModule):
    """
        Harvester retrieving event level information of reconstructed recorded data.
    """

    def __init__(self,
                 output_file_name,
                 pxd_clusters_name='PXDClusters',
                 pxd_spacepoints_name="PXDSpacePoints",
                 svd_clusters_name="SVDClusters",
                 svd_spacepoints_name="SVDSpacePoints",
                 cdc_hits_name="CDCHits",
                 reco_tracks_name="RecoTracks",
                 cdc_reco_tracks_name="CDCRecoTracks",
                 svd_cdc_reco_tracks_name="SVDCDCRecoTracks",
                 svd_reco_tracks_name="SVDRecoTracks",
                 pxd_reco_tracks_name="PXDRecoTracks"
                 ):
        """Expecting a name for the output file"""
        super().__init__(foreach='EventMetaData',
                         output_file_name=output_file_name,
                         )

        #: cached value of the PXDClusters StoreArray
        self.pxd_clusters_name = pxd_clusters_name
        #: cached value of the PXDSpacePoints StoreArray
        self.pxd_spacepoints_name = pxd_spacepoints_name
        #: cached value of the SVDClusters StoreArray
        self.svd_clusters_name = svd_clusters_name
        #: cached value of the SVDSpacePoints StoreArray
        self.svd_spacepoints_name = svd_spacepoints_name
        #: cached value of the CDCHits StoreArray
        self.cdc_hits_name = cdc_hits_name

        #: cached value of the RecoTracks StoreArray
        self.reco_tracks_name = reco_tracks_name
        #: cached value of the CDCRecoTracks StoreArray
        self.cdc_reco_tracks_name = cdc_reco_tracks_name
        #: cached value of the SVDCDCRecoTracks StoreArray
        self.svd_cdc_reco_tracks_name = svd_cdc_reco_tracks_name
        #: cached value of the SVDRecoTracks StoreArray
        self.svd_reco_tracks_name = svd_reco_tracks_name
        #: cached value of the PXDRecoTracks StoreArray
        self.pxd_reco_tracks_name = pxd_reco_tracks_name

    def peel(self, event_meta_data):
        """Extract and store counts of *RecoTracks' and hits, clusters, spacepoints"""

        event_crops = peelers.peel_event_info(event_meta_data)

        event_level_tracking_info = Belle2.PyStoreObj("EventLevelTrackingInfo")
        event_level_tracking_info_crops = peelers.peel_event_level_tracking_info(event_level_tracking_info)

        number_of_hits = dict(
            **peelers.peel_store_array_size(self.pxd_clusters_name),
            **peelers.peel_store_array_size(self.pxd_spacepoints_name),
            **peelers.peel_store_array_size(self.svd_clusters_name),
            **peelers.peel_store_array_size(self.svd_spacepoints_name),
            **peelers.peel_store_array_size(self.cdc_hits_name),
        )

        number_of_tracks = dict(
            **peelers.peel_store_array_size(self.reco_tracks_name),
            **peelers.peel_store_array_size(self.cdc_reco_tracks_name),
            **peelers.peel_store_array_size(self.svd_cdc_reco_tracks_name),
            **peelers.peel_store_array_size(self.svd_reco_tracks_name),
            **peelers.peel_store_array_size(self.pxd_reco_tracks_name),
        )

        module_list = ["SegmentNetworkProducer", "TrackFinderVXDCellOMat"]
        module_stats = peelers.peel_module_statistics(module_list)

        return dict(**event_crops,
                    **event_level_tracking_info_crops,
                    **number_of_hits,
                    **number_of_tracks,
                    **module_stats,
                    )

    #: Save a tree of all collected variables in a sub folder
    save_tree = refiners.SaveTreeRefiner()


class TrackInfoHarvester(harvesting.HarvestingModule):
    """
        Harvester retrieving track level information of reconstructed recorded data.
    """

    def __init__(self, output_file_name,
                 reco_tracks_name="RecoTracks",
                 svd_cdc_reco_tracks_name="SVDCDCRecoTracks",
                 svd_reco_tracks_name="SVDRecoTracks",
                 sp_track_cands_name="SPTrackCands"
                 ):
        """Expecting a name for the output file and the name of the RecoTracks StoreArray
           to operate on. The latter defaults to 'RecoTracks'"""
        super().__init__(foreach=reco_tracks_name,
                         output_file_name=output_file_name)

        #: cached value of the SVDCDCRecoTracks StoreArray
        self.svd_cdc_reco_tracks_name = svd_cdc_reco_tracks_name
        #: cached value of the SVDRecoTracks StoreArray
        self.svd_reco_tracks_name = svd_reco_tracks_name
        #: cached value of the SPTrackCands StoreArray
        self.sp_track_cands_name = sp_track_cands_name

    def peel(self, reco_track):
        """Extract and store information about each RecoTrack"""

        event_meta_data = Belle2.PyStoreObj("EventMetaData")
        event_crops = peelers.peel_event_info(event_meta_data)

        # Information on the store array
        store_array_info = peelers.peel_store_array_info(reco_track)

        # General information on the track
        reco_track_hit_content = peelers.peel_reco_track_hit_content(reco_track)
        reco_track_seed = peelers.peel_reco_track_seed(reco_track)
        fit_status = peelers.peel_fit_status(reco_track)

        # Information on the track fit result
        related_belle2_track = reco_track.getRelated("Tracks")
        if related_belle2_track:
            track_fit_status = peelers.peel_track_fit_result(
                related_belle2_track.getTrackFitResultWithClosestMass(Belle2.Const.pion))
        else:
            track_fit_status = peelers.peel_track_fit_result(None)

        vxdtf2_was_involved = False
        svd_cdc_reco_track = reco_track.getRelated(self.svd_cdc_reco_tracks_name)
        if svd_cdc_reco_track:
            svd_reco_track = svd_cdc_reco_track.getRelated(self.svd_reco_tracks_name)
            if svd_reco_track:
                svd_sptcs = svd_reco_track.getRelated(self.sp_track_cands_name)
                if svd_sptcs:
                    vxdtf2_was_involved = True

        return dict(**reco_track_hit_content,
                    **reco_track_seed,
                    **fit_status,
                    **track_fit_status,
                    **store_array_info,
                    vxdtf2_was_involved=vxdtf2_was_involved,
                    **event_crops,
                    )

    #: Save a tree of all collected variables in a sub folder
    save_tree = refiners.SaveTreeRefiner()


class HitInfoHarvester(harvesting.HarvestingModule):
    """
        Harvester retrieving hit level information of reconstructed recorded data.
    """

    def __init__(self, output_file_name, reco_tracks_name="RecoTracks"):
        """Expecting a name for the output file and the name of the RecoTracks StoreArray
           to operate on. The latter dafaults to 'RecoTracks'"""
        super().__init__(foreach=reco_tracks_name,
                         output_file_name=output_file_name)

    def peel(self, reco_track):
        """Extract and store information about each RecoTrack's hits"""

        # Event Info
        event_meta_data = Belle2.PyStoreObj("EventMetaData")
        event_crops = peelers.peel_event_info(event_meta_data)

        # Information on the store array
        store_array_info = peelers.peel_store_array_info(reco_track)

        # Getting residuals for each hit of the RecoTrack
        for hit_info in reco_track.getRelationsWith("RecoHitInformations"):
            layer = np.float("nan")
            if hit_info.getTrackingDetector() == Belle2.RecoHitInformation.c_SVD:
                hit = hit_info.getRelated("SVDClusters")
                layer = hit.getSensorID().getLayerNumber()
            if hit_info.getTrackingDetector() == Belle2.RecoHitInformation.c_PXD:
                hit = hit_info.getRelated("PXDClusters")
                layer = hit.getSensorID().getLayerNumber()
            if hit_info.getTrackingDetector() == Belle2.RecoHitInformation.c_CDC:
                hit = hit_info.getRelated("CDCHits")
                layer = hit.getISuperLayer()

            if hit_info.useInFit() and reco_track.hasTrackFitStatus():
                track_point = reco_track.getCreatedTrackPoint(hit_info)
                fitted_state = track_point.getFitterInfo()
                if fitted_state:
                    try:
                        res_info = fitted_state.getResidual()
                        res = np.sqrt(res_info.getState().Norm2Sqr())

                        yield dict(**store_array_info,
                                   **event_crops,
                                   residual=res,
                                   tracking_detector=hit_info.getTrackingDetector(),
                                   use_in_fit=hit_info.useInFit(),
                                   layer_number=layer
                                   )
                    except BaseException:
                        pass

    #: Save a tree of all collected variables in a sub folder
    save_tree = refiners.SaveTreeRefiner()
