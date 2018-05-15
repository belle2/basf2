import basf2

from ROOT import Belle2, TVector3, genfit

from tracking.harvest.harvesting import HarvestingModule
import tracking.harvest.peelers as peelers
import tracking.harvest.refiners as refiners


class EventInfoHarvester(HarvestingModule):
    def __init__(self, output_file_name):
        super(EventInfoHarvester, self).__init__(foreach='EventMetaData', output_file_name=output_file_name)

    def peel(self, event_meta_data):
        # Event Info
        event_crops = peelers.peel_event_info(event_meta_data)

        # Retrieving flag indicating if the vxdtf2 execution was aborted
        event_level_tracking_info = Belle2.PyStoreObj("EventLevelTrackingInfo")
        if event_level_tracking_info:
            has_vxdtf2_failure_flag = event_level_tracking_info.hasVXDTF2AbortionFlag()
        else:
            has_vxdtf2_failure_flag = float("nan")

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


class TrackInfoHarvester(HarvestingModule):
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


class HitInfoHarvester(HarvestingModule):
    def __init__(self, output_file_name, reco_tracks_name="RecoTracks"):
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

    save_tree = refiners.SaveTreeRefiner()


CENTRAL_332_DB = "332_COPY-OF_GT_gen_prod_004.11_Master-20171213-230000"
SECTORMAP_DB = "/home/belle2/lueck/releases/myHead/workdir_SMPhase2_zVertex1cm/localdb_noTiming/SM_phase2_1cmZshift_noTiming.txt"
ALIGNMENT_DB = "/home/belle2/bilka/alignment/phase2/caf_output_init/beamspot/outputdb/database.txt"
MAGNETIC_FIELD_DB = "MagneticFieldPhase2-2018-04"


class ReconstructionTask(GridControlMixin, SimplifiedOutputBasf2Task):
    global_tags = Parameter()
    input_file_name = Parameter()

    def create_path(self):
        basf2.reset_database()
        basf2.use_database_chain()

        if self.global_tags == "332_COPY-OF_GT_gen_prod_004.11_Master-20171213-230000_magnetic_thomas_sectormap_alignment":
            global_tags = [CENTRAL_332_DB, MAGNETIC_FIELD_DB, SECTORMAP_DB, ALIGNMENT_DB]
        elif self.global_tags == "332_COPY-OF_GT_gen_prod_004.11_Master-20171213-230000_thomas_sectormap_alignment":
            global_tags = [CENTRAL_332_DB, SECTORMAP_DB, ALIGNMENT_DB]
        elif self.global_tags == "332_COPY-OF_GT_gen_prod_004.11_Master-20171213-230000_thomas_sectormap":
            global_tags = [CENTRAL_332_DB, SECTORMAP_DB]
        else:
            raise ValueError(self.global_tags)

        for global_tag in global_tags:
            if global_tag.startswith("/"):
                basf2.use_local_database(global_tag)
            else:
                basf2.use_central_database(global_tag)

        path = basf2.create_path()

        path.add_module("EventInfoSetter")
        path.add_module(EventInfoHarvester(output_file_name=self.get_output_file_name('eventLevelInformation.root')))

        return path


class CombinedReconstructionTask(HaddTask):
    global_tags = Parameter()

    def requires(self):
        for input_file_name in recorded_data(pattern="beam*.root", basedir="dummy"):
            yield ReconstructionTask(input_file_name=input_file_name, global_tags=self.global_tags)


class MainTask(WrapperTask):
    def requires(self):
        for global_tags in ["332_COPY-OF_GT_gen_prod_004.11_Master-20171213-230000_magnetic_thomas_sectormap_alignment",
                            "332_COPY-OF_GT_gen_prod_004.11_Master-20171213-230000_thomas_sectormap_alignment",
                            "332_COPY-OF_GT_gen_prod_004.11_Master-20171213-230000_thomas_sectormap"]:
            yield CombinedReconstructionTask(global_tags=global_tags)


if __name__ == '__main__':
    create_study([MainTask()], backend="host", use_gridcontrol=True, in_flight=4)
