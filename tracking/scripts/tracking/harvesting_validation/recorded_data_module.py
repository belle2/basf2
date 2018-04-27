#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2

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
        super(EventInfoHarvester, self).__init__(foreach='EventMetaData',
                                                 output_file_name=output_file_name,
                                                 )

        self.pxd_clusters_name = pxd_clusters_name
        self.pxd_spacepoints_name = pxd_spacepoints_name
        self.svd_clusters_name = svd_clusters_name
        self.svd_spacepoints_name = svd_spacepoints_name
        self.cdc_hits_name = cdc_hits_name

        self.reco_track_name = reco_tracks_name
        self.cdc_reco_tracks_name = cdc_reco_tracks_name
        self.svd_cdc_reco_tracks_name = svd_cdc_reco_tracks_name
        self.svd_reco_tracks_name = svd_reco_tracks_name
        self.pxd_reco_tracks_name = pxd_reco_tracks_name

    def peel(self, event_meta_data):
        event_crops = peelers.peel_event_info(event_meta_data)

        event_level_tracking_info = Belle2.PyStoreObj("EventLevelTrackingInfo")
        event_level_tracking_info_crops = peelers.peel_event_level_tracking_info(event_level_tracking_info)

        number_of_hits = dict(
            pxd_clusters_size=peelers.peel_store_array_size(Belle2.PyStoreArray(self.pxd_clusters_name)),
            pxd_spacepoints_size=peelers.peel_store_array_size(Belle2.PyStoreArray(self.pxd_spacepoints_name)),
            svd_clusters_size=peelers.peel_store_array_size(Belle2.PyStoreArray(self.svd_clusters_name)),
            svd_spacepoints_size=peelers.peel_store_array_size(Belle2.PyStoreArray(self.svd_spacepoints_name)),
            cdc_hits_size=peelers.peel_store_array_size(Belle2.PyStoreArray(self.cdc_hits_name)),
        )

        number_of_tracks = dict(
            tracks_size=peelers.peel_store_array_size(Belle2.PyStoreArray(self.reco_tracks_name)),
            cdc_tracks_size=peelers.peel_store_array_size(Belle2.PyStoreArray(self.cdc_reco_tracks_name)),
            svd_cdc_tracks_size=peelers.peel_store_array_size(Belle2.PyStoreArray(self.svd_cdc_reco_tracks_name)),
            svd_tracks_size=peelers.peel_store_array_size(Belle2.PyStoreArray(self.svd_reco_tracks_name)),
            pxd_tracks_size=peelers.peel_store_array_size(Belle2.PyStoreArray(self.pxd_reco_tracks_name)),
        )

        module_list = ["SegmentNetworkProducer", "TrackFinderVXDCellOMat"]
        module_stats = peelers.peel_module_statistics(module_list)

        return dict(**event_crops,
                    **event_level_tracking_info_crops,
                    **number_of_hits,
                    **number_of_tracks,
                    **module_stats,
                    )

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
           to operate on. The latter dafaults to 'RecoTracks'"""
        super(TrackInfoHarvester, self).__init__(foreach=reco_tracks_name,
                                                 output_file_name=output_file_name)

        self.svd_cdc_reco_tracks_name = svd_cdc_reco_tracks_name
        self.svd_reco_tracks_name = svd_reco_tracks_name
        self.sp_tracks_cands_name = sp_track_cands_name

    def peel(self, reco_track):
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

    save_tree = refiners.SaveTreeRefiner()


class HitInfoHarvester(harvesting.HarvestingModule):
    """
        Harvester retrieving hit level information of reconstructed recorded data.
    """

    def __init__(self, output_file_name, reco_tracks_name="RecoTracks"):
        """Expecting a name for the output file and the name of the RecoTracks StoreArray
           to operate on. The latter dafaults to 'RecoTracks'"""
        super(HitInfoHarvester, self).__init__(foreach=reco_tracks_name,
                                               output_file_name=output_file_name)

    def peel(self, reco_track):
        # Event Info
        event_meta_data = Belle2.PyStoreObj("EventMetaData")
        event_crops = peelers.peel_event_info(event_meta_data)

        # Information on the store array
        store_array_info = peelers.peel_store_array_info(reco_track)

        # Getting residuals for each hit of the RecoTrack
        for hit_info in reco_track.getRelationsWith("RecoHitInformations"):
            yield peelers.peel_hit_infromation(hit_info, reco_track)

    save_tree = refiners.SaveTreeRefiner()
