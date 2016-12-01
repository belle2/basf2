import basf2
import ROOT
from ROOT import Belle2

from ROOT import gSystem
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')

from tracking.harvest.harvesting import HarvestingModule
from tracking.harvest import refiners
import logging
import sys

from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule


class FitValidationModule(HarvestingModule):

    def __init__(
            self,
            mc_track_cands_store_array_name,
            legendre_track_cand_store_array_name,
            output_file_name):
        super(
            FitValidationModule,
            self).__init__(
            foreach=legendre_track_cand_store_array_name,
            output_file_name=output_file_name)

        self.mc_track_cands_store_array_name = mc_track_cands_store_array_name
        self.legendre_track_cand_store_array_name = legendre_track_cand_store_array_name

        # Fitters
        self.circle_fitter = Belle2.TrackFindingCDC.CDCRiemannFitter.getFitter()
        self.fast_fitter = Belle2.TrackFindingCDC.TrackFitter()

    def prepare(self):
        self.cdcHits = Belle2.PyStoreArray("CDCHits")
        return HarvestingModule.prepare(self)

    def peel(self, legendre_track_cand):

        cdc_hit_store_array = self.cdcHits

        observations_variance = Belle2.TrackFindingCDC.CDCObservations2D()
        observations = Belle2.TrackFindingCDC.CDCObservations2D()
        hits = ROOT.std.vector("Belle2::TrackFindingCDC::TrackHit*")()

        cdc_hit_ids = legendre_track_cand.getHitIDs(Belle2.Const.CDC)
        for cdc_hit_id in cdc_hit_ids:
            cdc_hit = cdc_hit_store_array[cdc_hit_id]

            # We will only use the axial hits here as the FastFit can only process axial hits too
            if cdc_hit.getISuperLayer() % 2 == 0:
                cdc_wire_hit = Belle2.TrackFindingCDC.CDCWireHit(cdc_hit)

                wireRefPos2D = cdc_wire_hit.getRefPos2D()
                drift_length = cdc_wire_hit.getRefDriftLength()
                observations.append(wireRefPos2D.x(), wireRefPos2D.y(), 0, 1 / abs(drift_length))
                hits.push_back(Belle2.TrackFindingCDC.TrackHit(cdc_wire_hit))

        # Viktors method
        track_par = ROOT.std.pair("double, double")()
        ref_point = ROOT.std.pair("double, double")()
        # Careful: viktors result is a reduced chi2
        viktor_chi2 = self.fast_fitter.fitTrackCandidateFast(hits, track_par, ref_point, False) * (hits.size() - 4)

        # Riemann without drift variances
        trajectory2D = Belle2.TrackFindingCDC.CDCTrajectory2D()
        self.circle_fitter.update(trajectory2D, observations)
        riemann_chi2 = trajectory2D.getChi2()

        return_dict = dict(
            riemann_chi2=riemann_chi2,
            viktor_chi2=viktor_chi2,
        )

        return return_dict

    save_tree = refiners.save_tree(folder_name="tree")


from tracking.run.event_generation import StandardEventGenerationRun
from tracking import modules


class FitValidation(StandardEventGenerationRun):

    def create_path(self):

        main_path = super(FitValidation, self).create_path()

        main_path.add_module("WireHitPreparer")

        main_path.add_module(modules.CDCMCFinder())

        main_path.add_module(FitValidationModule(mc_track_cands_store_array_name="MCTrackCands",
                                                 legendre_track_cand_store_array_name="MCTrackCands",
                                                 output_file_name="fit_validation.root"))

        return main_path


def main():
    run = FitValidation()
    run.configure_and_execute_from_commandline()

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
