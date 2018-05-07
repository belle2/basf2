#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import numpy as np

from ROOT import Belle2
import basf2

import rawdata

from tracking import add_hit_preparation_modules, add_track_finding, add_track_fit_and_track_creator

import tracking.harvest.harvesting as harvesting
import tracking.harvest.peelers as peelers
import tracking.harvest.refiners as refiners


class EventInfoHarvester(harvesting.HarvestingModule):
    def __init__(self, output_file_name):
        super(EventInfoHarvester, self).__init__(foreach='EventMetaData', output_file_name=output_file_name)

    def peel(self, event_meta_data):
        # Event Info
        event_crops = peelers.peel_event_info(event_meta_data)

        # Retrieving flag indicating if the vxdtf2 execution was aborted
        event_level_tracking_info = Belle2.PyStoreObj("EventLevelTrackingInfo")
        has_vxdtf2_failure_flag = event_level_tracking_info.hasVXDTF2AbortionFlag()

        # Number of PXD and SVD Clusters/SpacePoints as well as CDC hits
        pxd_clusters = Belle2.PyStoreArray("PXDClusters")
        pxd_sps = Belle2.PyStoreArray("PXDSpacePoints")
        svd_clusters = Belle2.PyStoreArray("SVDClusters")
        svd_sps = Belle2.PyStoreArray("SVDSpacePoints")
        cdc_hits = Belle2.PyStoreArray("CDCHits")

        number_of_hits = dict(
            pxd_clusters_size=pxd_clusters.getEntries() if pxd_clusters else 0,
            pxd_spacepoints_size=pxd_sps.getEntries() if pxd_sps else 0,
            svd_clusters_size=svd_clusters.getEntries() if svd_clusters else 0,
            svd_spacepoints_size=svd_sps.getEntries() if svd_sps else 0,
            cdc_hits_size=cdc_hits.getEntries() if cdc_hits else 0,
        )

        # Number of tracks (reco, cdc, cdcsvd, svd, pxd)
        tracks = Belle2.PyStoreArray("RecoTracks")
        cdc_tracks = Belle2.PyStoreArray("CDCRecoTracks")
        svd_cdc_tracks = Belle2.PyStoreArray("SVDCDCRecoTracks")
        svd_tracks = Belle2.PyStoreArray("SVDRecoTracks")
        pxd_tracks = Belle2.PyStoreArray("PXDRecoTracks")
        number_of_tracks = dict(
            tracks_size=tracks.getEntries() if tracks else 0,
            cdc_tracks_size=cdc_tracks.getEntries() if cdc_tracks else 0,
            svd_cdc_tracks_size=svd_cdc_tracks.getEntries() if svd_cdc_tracks else 0,
            svd_tracks_size=svd_tracks.getEntries() if svd_tracks else 0,
            pxd_tracks_size=pxd_tracks.getEntries() if pxd_tracks else 0,
        )

        # Information about the run time and memory consumption of selected VXDTF2 modules
        stats = {}
        module_stats = basf2.statistics.modules
        for module in module_stats:
            if module.name in ["SegmentNetworkProducer", "TrackFinderVXDCellOMat"]:
                stats[str(module.name) + "_mem"] = module.memory_sum(basf2.statistics.EVENT)
                stats[str(module.name) + "_time"] = module.time_sum(basf2.statistics.EVENT)

        return dict(has_vxdtf2_failure_flag=has_vxdtf2_failure_flag,
                    **number_of_hits,
                    **number_of_tracks,
                    **event_crops,
                    **stats,
                    )

    save_tree = refiners.SaveTreeRefiner()


class TrackInfoHarvester(harvesting.HarvestingModule):
    def __init__(self, output_file_name, reco_tracks_name="RecoTracks"):
        super(TrackInfoHarvester, self).__init__(foreach=reco_tracks_name,
                                                 output_file_name=output_file_name)

    def peel(self, reco_track):
        # Event Info
        event_meta_data = Belle2.PyStoreObj("EventMetaData")
        event_crops = peelers.peel_event_info(event_meta_data)

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

        # Information on the store array
        store_array_info = peelers.peel_store_array_info(reco_track)

        vxdtf2_was_involved = False
        svd_cdc_reco_track = reco_track.getRelated("SVDCDCRecoTracks")
        if svd_cdc_reco_track:
            svd_reco_track = svd_cdc_reco_track.getRelated("SVDRecoTracks")
            if svd_reco_track:
                svd_sptcs = svd_reco_track.getRelated("SPTrackCands")
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
    def __init__(self, output_file_name, reco_tracks_name="RecoTracks"):
        super(HitInfoHarvester, self).__init__(foreach=reco_tracks_name,
                                               output_file_name=output_file_name)

    def peel(self, reco_track):
        nan = np.float("nan")
        # Event Info
        event_meta_data = Belle2.PyStoreObj("EventMetaData")
        event_crops = peelers.peel_event_info(event_meta_data)

        # Information on the store array
        store_array_info = peelers.peel_store_array_info(reco_track)

        # Getting residuals for each hit of the RecoTrack
        for hit_info in reco_track.getRelationsWith("RecoHitInformations"):
            hit_time = nan
            layer = nan
            if hit_info.getTrackingDetector() == Belle2.RecoHitInformation.c_SVD:
                hit = hit_info.getRelated("SVDClusters")
                hit_time = hit.getClsTime()
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
                        res_state = fitted_state.getResidual().getState()
                        res = np.sqrt(res_state.Norm2Sqr())

                        residual_x = residual_y = nan
                        if res_state.GetNoElements() == 2:
                            residual_x = res_state[0]
                            residual_y = res_state[1]

                        weights = fitted_state.getWeights()

                        yield dict(**store_array_info,
                                   **event_crops,
                                   residual=res,
                                   residual_x=residual_x,
                                   residual_y=residual_y,
                                   weight=max(weights),
                                   tracking_detector=hit_info.getTrackingDetector(),
                                   use_in_fit=hit_info.useInFit(),
                                   hit_time=hit_time,
                                   layer_number=layer,
                                   )
                    except BaseException:
                        pass

    save_tree = refiners.SaveTreeRefiner()


def get_output_file_name(file_name):
    input_file_names = os.environ["FILE_NAMES"].split()
    assert len(input_file_names) == 1
    input_file_name = input_file_names[0]
    input_file_name = os.path.splitext(os.path.basename(input_file_name))[0]

    return input_file_name + "_" + file_name


if __name__ == "__main__":
    basf2.reset_database()
    basf2.use_central_database(os.environ["BASF2_GDT"])

    path = basf2.create_path()

    input_file_names = os.environ["FILE_NAMES"].split()
    print(f"Using input files {input_file_names}")

    path.add_module("RootInput", inputFileNames=input_file_names)

    path.add_module("Gearbox")
    path.add_module("Geometry", useDB=True)

    rawdata.add_unpackers(path)

    add_hit_preparation_modules(path)
    path.add_module('SetupGenfitExtrapolation', energyLossBrems=False, noiseBrems=False)

    add_track_finding(path, svd_ckf_mode="VXDTF2_before_with_second_ckf", prune_temporary_tracks=False)

    add_track_fit_and_track_creator(path)

    # Resetting MinClusterTime parameter to turn of cut on cluster time
    for m in path.modules():
        if m.name() == "SVDSpacePointCreator":
            m.param("MinClusterTime", -999)

    path.add_module("RootOutput", outputFileName=get_output_file_name("reconstructed.root"))

    path.add_module(TrackInfoHarvester(output_file_name=get_output_file_name('trackLevelInformation.root')))
    path.add_module(HitInfoHarvester(output_file_name=get_output_file_name('hitLevelInformation.root')))
    path.add_module(EventInfoHarvester(output_file_name=get_output_file_name('eventLevelInformation.root')))

    basf2.print_path(path)
    basf2.process(path)
    print(basf2.statistics)
