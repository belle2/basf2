import basf2
import ROOT
from ROOT import Belle2

from ROOT import gSystem
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')

from trackfinderoutputcombiner.combinerValidation import CombinerTrackFinderRun, AddValidationMethod
from trackfinderoutputcombiner.combination_checker import CombinationChecker
from tracking.validation.harvesting import HarvestingModule
from tracking.validation import refiners
import logging
import sys

from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule
from trackfindingcdc.cdcdisplay.svgdrawing.attributemaps import CDCSegmentColorMap
CDCSegmentColorMap.bkgSegmentColor = "black"

try:
    import root_pandas
except ImportError:
    print "do a pip install git+https://github.com/ibab/root_pandas"
import pandas
import matplotlib.pyplot as plt
import seaborn as sb
import numpy as np


class SegmentFakeRatesModule(HarvestingModule):

    def __init__(
            self,
            local_track_cands_store_array_name,
            mc_track_cands_store_array_name,
            legendre_track_cand_store_array_name,
            output_file_name):
        super(
            SegmentFakeRatesModule,
            self).__init__(
            foreach=local_track_cands_store_array_name,
            output_file_name=output_file_name)

        self.mc_track_cands_store_array_name = mc_track_cands_store_array_name
        self.legendre_track_cand_store_array_name = legendre_track_cand_store_array_name

        self.mc_track_matcher_local = Belle2.TrackMatchLookUp(self.mc_track_cands_store_array_name, self.foreach)
        self.mc_track_matcher_legendre = Belle2.TrackMatchLookUp(
            self.mc_track_cands_store_array_name,
            legendre_track_cand_store_array_name)

    def prepare(self):
        self.cdcHits = Belle2.PyStoreArray("CDCHits")
        return HarvestingModule.prepare(self)

    def peel(self, local_track_cand):
        mc_track_matcher_local = self.mc_track_matcher_local
        mc_track_matcher_legendre = self.mc_track_matcher_legendre

        is_background = mc_track_matcher_local.isBackgroundPRTrackCand(local_track_cand)
        is_ghost = mc_track_matcher_local.isGhostPRTrackCand(local_track_cand)
        is_matched = mc_track_matcher_local.isMatchedPRTrackCand(local_track_cand)
        is_clone = mc_track_matcher_local.isClonePRTrackCand(local_track_cand)
        is_clone_or_matched = is_matched or is_clone
        hit_purity = abs(mc_track_matcher_local.getRelatedPurity(local_track_cand))

        # Stereo Track?
        first_cdc_hit_id = local_track_cand.getHitIDs(Belle2.Const.CDC)[0]
        first_cdc_hit = self.cdcHits[first_cdc_hit_id]
        is_stereo = first_cdc_hit.getISuperLayer() % 2 == 1

        has_partner = np.NaN
        hit_purity_of_partner = np.NaN
        hit_efficiency_of_partner = np.NaN
        mc_track_pt = np.NaN

        if is_clone_or_matched:
            related_mc_track_cand = mc_track_matcher_local.getRelatedMCTrackCand(local_track_cand)
            has_partner = (mc_track_matcher_legendre.isMatchedMCTrackCand(related_mc_track_cand) or
                           mc_track_matcher_legendre.isMergedMCTrackCand(related_mc_track_cand))
            mc_track_pt = related_mc_track_cand.getMomSeed().Pt()
            if has_partner:
                partner_legendre_track_cand = mc_track_matcher_legendre.getRelatedPRTrackCand(related_mc_track_cand)
                hit_purity_of_partner = abs(mc_track_matcher_legendre.getRelatedPurity(partner_legendre_track_cand))
                hit_efficiency_of_partner = abs(mc_track_matcher_legendre.getRelatedEfficiency(related_mc_track_cand))

        return dict(is_background=is_background,
                    is_ghost=is_ghost,
                    is_matched=is_matched,
                    is_clone=is_clone,
                    is_clone_or_matched=is_clone_or_matched,
                    is_stereo=is_stereo,
                    mc_track_pt=mc_track_pt,
                    hit_purity=hit_purity,
                    has_partner=has_partner,
                    hit_purity_of_partner=hit_purity_of_partner,
                    hit_efficiency_of_partner=hit_efficiency_of_partner)

    save_tree = refiners.save_tree(folder_name="tree")


class Filler(basf2.Module):

    def event(self):
        Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance().fill()


class SegmentFakeRates(CombinerTrackFinderRun, AddValidationMethod):

    display_module = CDCSVGDisplayModule(output_folder="tmp/mc/", interactive=False)
    display_module.draw_segments_mctrackid = False
    display_module.draw_hits = True
    display_module.draw_gftrackcands = True
    display_module.draw_gftrackcand_trajectories = True
    display_module.track_cands_store_array_name = "MCTrackCands"

    display_module2 = CDCSVGDisplayModule(output_folder="tmp/segments/", interactive=False)
    display_module2.draw_hits = True
    display_module2.draw_gftrackcands = True
    display_module2.draw_gftrackcand_trajectories = True
    display_module2.track_cands_store_array_name = "LocalTrackCands"

    display_module_segments = CDCSVGDisplayModule(output_folder="tmp/segments/", interactive=False)
    display_module_segments.draw_hits = True
    display_module_segments.draw_gftrackcand_trajectories = True
    display_module_segments.draw_gftrackcands = True
    display_module_segments.track_cands_store_array_name = "LegendreTrackCands"
    display_module_segments.draw_segments_mctrackid = True

    def create_path(self):

        # Turn of combining
        self.do_combining = False
        self.use_segment_quality_check = False
        self.stereo_assignment = False

        main_path = super(SegmentFakeRates, self).create_path()

        # main_path.add_module(self.create_matcher_module(self.legendre_track_cands_store_array_name))
        # main_path.add_module(self.create_matcher_module(self.local_track_cands_store_array_name))

        # main_path.add_module(SegmentFakeRatesModule(local_track_cands_store_array_name=self.local_track_cands_store_array_name,
        #                                            mc_track_cands_store_array_name=self.mc_track_cands_store_array_name,
        #                                            legendre_track_cand_store_array_name=self.legendre_track_cands_store_array_name,
        #                                            output_file_name="segment_fake_rates.root"))

        # main_path.add_module(self.display_module)
        # main_path.add_module(self.display_module2)

        main_path.add_module(Filler())
        main_path.add_module(self.display_module_segments)
        cdc_drawing_module = basf2.register_module("CDCNiceDrawing")
        cdc_drawing_module.param({
            'StoreDirectory': "CDCNiceDrawing",
            'DrawAlsoDifference': False,
            'DrawMCSignal': False,
            'TrackCandColName': "MCTrackCands",
            'MCTrackCandColName': "MCTrackCands",
            'HitColName': "CDCHits"
        })

        main_path.add_module(cdc_drawing_module)

        return main_path


def main():
    run = SegmentFakeRates()
    run.configure_and_execute_from_commandline()


def plot():
    data = root_pandas.read_root("segment_fake_rates.root", tree_key="tree/SegmentFakeRatesModule_tree")
    data["is_clone_and_has_partner"] = (data.is_clone == 1) & (data.has_partner == 1)
    data.hist()
    plt.savefig("segment_fake_rates.pdf")

    # Print some results
    stereo_segments = data.loc[data.is_stereo == 1]
    axial_segments = data.loc[data.is_stereo == 0]

    def print_column_rate(column):
        print str(column), "stereo:", stereo_segments[column].mean()
        print str(column), "axial:", axial_segments[column].mean()
        print str(column), data[column].mean()

    print "stereo:", stereo_segments["is_stereo"].count()
    print "axial:", axial_segments["is_stereo"].count()

    print_column_rate("is_clone_or_matched")
    print_column_rate("is_ghost")
    print_column_rate("is_background")
    print_column_rate("has_partner")
    print_column_rate("is_clone")
    print_column_rate("is_clone_and_has_partner")

    # Plot the pts
    plt.clf()
    ax = plt.subplot(211)
    axial_segments.mc_track_pt.hist()
    plt.xlabel("pt for axial segments")
    plt.subplot(212, sharex=ax)
    stereo_segments.mc_track_pt.hist()
    plt.xlabel("pt for stereo segments")
    plt.savefig("mc_track_pt.pdf")

    plt.clf()
    ax = plt.subplot(211)
    axial_segments.loc[axial_segments.has_partner == 0].mc_track_pt.hist()
    plt.xlabel("pt for axial segments")
    plt.subplot(212, sharex=ax)
    stereo_segments.loc[stereo_segments.has_partner == 0].mc_track_pt.hist()
    plt.xlabel("pt for stereo segments")
    plt.savefig("mc_track_pt_new.pdf")

if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
    # plot()
