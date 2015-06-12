import basf2
import ROOT
from ROOT import Belle2

from ROOT import gSystem
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')

from trackfinderoutputcombiner.combinerValidation import CombinerTrackFinderRun, AddValidationMethod
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
