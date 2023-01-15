##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from pybasf2 import B2DEBUG
from ROOT import Belle2
import basf2 as b2


import torch
import numpy as np

from vtx_cdc_merger.interaction_network.NNet import NNetWrapper
import vtx_cdc_merger.var_set as var_set


class VTXCDCMergerExpert(b2.Module):
    """
    Module to compute cleaned and merged VTX+CDC tracks.

    The module updates the QulityIndicators of VXD and CDC tracks and the
    their relations. Please note that the final cleaned&merged RecoTrack
    StoreArray is produced later by the RelatedTracksCombiner.
    """

    def __init__(
            self,
            model_path,
            CDCRecoTrackColName="CDCRecoTrackColName",
            VXDRecoTrackColName="VXDRecoTrackColName",
    ):
        """Constructor"""
        super().__init__()

        #: cached name of the CDC RecoTracks StoreArray
        self.CDCRecoTrackColName = CDCRecoTrackColName

        #: cached name of VXD RecoTracks StoreArray
        self.VXDRecoTrackColName = VXDRecoTrackColName

        #: cached name of the model checkpoint folder
        self.model_path = model_path

        #: cached cut value: keep vxd tracks with classifier output above
        self.cut_vxd = 0.0

        #: cached cut value: keep cdc tracks with classifier output above
        self.cut_cdc = 0.0

        #: cached cut value: make relation for pair with classifier output above
        self.cut_links = 0.5

        #: cached cut value for making new vxd to cdc relations
        self.remove_relations = False

    def initialize(self):
        """
        Initialize the module.
        """

        self.nnet = NNetWrapper()
        self.nnet.load_checkpoint(self.model_path, 'best.pth.tar')

        cdcTracks = Belle2.PyStoreArray(self.CDCRecoTrackColName)
        vxdTracks = Belle2.PyStoreArray(self.VXDRecoTrackColName)
        vxdTracks.registerRelationTo(cdcTracks)

    def event(self):
        """
        Called for each event.
        """
        cdcTracks = Belle2.PyStoreArray(self.CDCRecoTrackColName)
        vxdTracks = Belle2.PyStoreArray(self.VXDRecoTrackColName)

        if self.remove_relations:
            # TODO need to cross check this code
            # Dissolve all currently found relations

            for cdcTrack in cdcTracks:
                B2DEBUG(9, "Look at CDCTrack at {}".format(cdcTrack.getArrayIndex()))

                relatedVXDRecoTracks = cdcTrack.getRelationsWith(self.VXDRecoTrackColName)

                nRelatedTracks = relatedVXDRecoTracks.size()
                offset = 0

                for index in range(nRelatedTracks):
                    # Need to remove the relation
                    relatedVXDRecoTracks.remove(index-offset)

                    offset += 1

            # Dissolve all currently found relations
            for vxdTrack in vxdTracks:
                B2DEBUG(9, "Look at VXDTrack at {}".format(vxdTrack.getArrayIndex()))

                relatedCDCRecoTracks = vxdTrack.getRelationsWith(self.CDCRecoTrackColName)

                nRelatedTracks = relatedCDCRecoTracks.size()
                offset = 0

                for index in range(nRelatedTracks):
                    # Need to remove the relation
                    relatedCDCRecoTracks.remove(index-offset)

                    offset += 1

        cdc_hits, cdc_tracks, vxd_hits, vxd_tracks = var_set.extract_event_data(cdcTracks=cdcTracks, vxdTracks=vxdTracks)

        vxd_hits = var_set.get_array(vxd_hits, var_set.scales_vxd_hits, use_scales=True)
        cdc_hits = var_set.get_array(cdc_hits, var_set.scales_cdc_hits, use_scales=True)
        vxd_tracks = var_set.get_array(vxd_tracks, var_set.scales_vxd_tracks, use_scales=True)
        cdc_tracks = var_set.get_array(cdc_tracks, var_set.scales_cdc_tracks, use_scales=True)

        vxd_hits = torch.from_numpy(vxd_hits).float()
        cdc_hits = torch.from_numpy(cdc_hits).float()
        vxd_tracks = torch.from_numpy(vxd_tracks).float()
        cdc_tracks = torch.from_numpy(cdc_tracks).float()

        pred_links, pred_tracks = self.nnet.predict(vxd_hits, cdc_hits, vxd_tracks, cdc_tracks)

        pred_tracks = np.atleast_1d(pred_tracks)

        # Apply the trackQI for VXD tracks
        n_vxd = len(vxdTracks)
        for i_vxd in range(n_vxd):
            if pred_tracks[i_vxd] < self.cut_vxd:
                vxdTracks[i_vxd].setQualityIndicator(0.0)
            else:
                vxdTracks[i_vxd].setQualityIndicator(1.0)

        # Apply the trackQI for CDC tracks
        n_cdc = len(cdcTracks)
        for i_cdc in range(n_cdc):
            if pred_tracks[n_vxd + i_cdc] < self.cut_cdc:
                cdcTracks[i_cdc].setQualityIndicator(0.0)
            else:
                cdcTracks[i_cdc].setQualityIndicator(1.0)

        # Make relations from VXD to CDC
        for i_links in range(pred_links.shape[0]):
            if pred_links[i_links] > self.cut_links:
                i_vxd = i_links // n_cdc
                i_cdc = i_links - i_vxd*n_cdc
                # TODO: do i need to check if relations exists?
                # -1 is the convention for "before the CDC track" in the related tracks combiner
                vxdTracks[i_vxd].addRelationTo(cdcTracks[i_cdc], -1)
