import basf2
import ROOT
from ROOT import Belle2

from ROOT import gSystem
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')

from trackfinderoutputcombiner.combinerValidation import MCTrackFinderRun, AddValidationMethod
from tracking.validation.harvesting import HarvestingModule
from tracking.validation import refiners
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

        # Matcher
        self.mc_track_matcher = Belle2.TrackMatchLookUp(mc_track_cands_store_array_name, legendre_track_cand_store_array_name)

    def prepare(self):
        self.cdcHits = Belle2.PyStoreArray("CDCHits")
        return HarvestingModule.prepare(self)

    def peel(self, legendre_track_cand):

        cdc_hit_store_array = self.cdcHits
        mc_track_matcher = self.mc_track_matcher

        observations_variance = Belle2.TrackFindingCDC.CDCObservations2D()
        observations = Belle2.TrackFindingCDC.CDCObservations2D()
        hits = ROOT.std.vector("Belle2::TrackFindingCDC::TrackHit*")()

        cdc_hit_ids = legendre_track_cand.getHitIDs(Belle2.Const.CDC)
        for cdc_hit_id in cdc_hit_ids:
            cdc_hit = cdc_hit_store_array[cdc_hit_id]

            # We will only use the axial hits here as the FastFit can only process axial hits too
            if cdc_hit.getISuperLayer() % 2 == 0:
                cdc_wire_hit = Belle2.TrackFindingCDC.CDCWireHit(cdc_hit)
                observations_variance.append(cdc_wire_hit)
                observations.append(cdc_wire_hit.getRefPos2D())
                hits.push_back(Belle2.TrackFindingCDC.TrackHit(cdc_hit, 0))

        # Fit the circle trajectory
        trajectory2D = Belle2.TrackFindingCDC.CDCTrajectory2D()
        self.circle_fitter.update(trajectory2D, observations_variance)
        riemann_chi2_variance = trajectory2D.getChi2()
        riemann_pt_with_variance = trajectory2D.getAbsMom2D()

        # Viktors method
        track_par = ROOT.std.pair("double, double")()
        ref_point = ROOT.std.pair("double, double")()
        # Careful: viktors result is a reduced chi2
        viktor_chi2 = self.fast_fitter.fitTrackCandidateFast(hits, track_par, ref_point, False) * (hits.size() - 4)
        viktor_pt = legendre_track_cand.getMomSeed().Pt()

        # Riemann without drift variances
        trajectory2D = Belle2.TrackFindingCDC.CDCTrajectory2D()
        self.circle_fitter.update(trajectory2D, observations)
        riemann_chi2 = trajectory2D.getChi2()
        riemann_pt = trajectory2D.getAbsMom2D()

        if mc_track_matcher.isMatchedPRTrackCand(legendre_track_cand):
            related_mc_track_cand = mc_track_matcher.getRelatedMCTrackCand(legendre_track_cand)
            mc_pt = related_mc_track_cand.getMomSeed().Pt()
        else:
            mc_pt = 0

        return_dict = dict(
            riemann_chi2=riemann_chi2,
            riemann_chi2_variance=riemann_chi2_variance,
            viktor_chi2=viktor_chi2,
            riemann_pt_with_variance=riemann_pt_with_variance,
            riemann_pt=riemann_pt,
            viktor_pt=viktor_pt,
            mc_pt=mc_pt)

        print return_dict
        return return_dict

    save_tree = refiners.save_tree(folder_name="tree")


class FitValidation(MCTrackFinderRun, AddValidationMethod):

    display_module = CDCSVGDisplayModule()
    display_module.draw_gftrackcand_trajectories = True

    def create_path(self):

        main_path = super(FitValidation, self).create_path()

        cdctracking = basf2.register_module('CDCLegendreTracking')
        cdctracking.set_log_level(basf2.LogLevel.WARNING)

        main_path.add_module(cdctracking)

        main_path.add_module(self.create_matcher_module("TrackCands"))

        main_path.add_module(FitValidationModule(mc_track_cands_store_array_name=self.mc_track_cands_store_array_name,
                                                 legendre_track_cand_store_array_name="TrackCands",
                                                 output_file_name="fit_validation.root"))

        # main_path.add_module(self.display_module)

        return main_path


def main():
    run = FitValidation()
    run.configure_and_execute_from_commandline()

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
