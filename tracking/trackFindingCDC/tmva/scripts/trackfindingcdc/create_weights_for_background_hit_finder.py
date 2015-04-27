#!/usr/bin/env python
# author Nils Braun, 2015
# This script is intended to create the weights needed for the TMVA in the BackgroundHitFinderModule.
# There should be no need to run this script again - only when something unexpected changes.

import basf2

from ROOT import gSystem
from matplotlib.gridspec import GridSpec
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')

import ROOT
from ROOT import Belle2
import numpy as np

import tracking.validation.harvesting as harvesting
import tracking.validation.refiners as refiners

from tracking.run.event_generation import StandardEventGenerationRun

try:
    import root_pandas
except ImportError:
    print "do a pip install git+https://github.com/ibab/root_pandas"
import pandas
import matplotlib.pyplot as plt
import seaborn as sb
sb.set_context("talk")
from matplotlib.gridspec import GridSpec

CONTACT = "ucddn@student.kit.edu"


class ClusterFilterValidationRun(StandardEventGenerationRun):
    segment_finder_module = basf2.register_module("SegmentFinderCDCFacetAutomatonDev")
    segment_finder_module.param({
        "WriteClusters": True,
        "FacetFilter": "none",
        "FacetNeighborChooser": "none",
    })

    py_profile = False
    output_file_name = "temp.root"  # Specification for BrowseTFileOnTerminateRunMixin

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(ClusterFilterValidationRun, self).create_path()

        segment_finder_module = self.get_basf2_module(self.segment_finder_module)
        main_path.add_module(segment_finder_module)

        # main_path.add_module(AxialStereoPairFitterModule())
        validation_module = ClusterFilterValidationModule(output_file_name=self.output_file_name)
        if self.py_profile:
            main_path.add_module(metamodules.PyProfilingModule(validation_module))
        else:
            main_path.add_module(validation_module)

        return main_path


class ClusterFilterValidationModule(harvesting.HarvestingModule):

    """Module to collect information about the facet creation cuts and compose validation plots on terminate."""

    def __init__(self, output_file_name):
        super(ClusterFilterValidationModule, self).__init__(foreach="CDCWireHitClusterVector",
                                                            output_file_name=output_file_name)

        self.mc_hit_lookup = Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance()
        self.superlayer_centers = list()

    def initialize(self):
        super(ClusterFilterValidationModule, self).initialize()

    def prepare(self):
        self.mc_hit_lookup.fill()

        # fill super layer look up
        wireTopology = Belle2.TrackFindingCDC.CDCWireTopology.getInstance()

        self.superlayer_centers = [
            0.5 *
            (
                wireTopology.getWireSuperLayer(superlayerID).getInnerCylindricalR() +
                wireTopology.getWireSuperLayer(superlayerID).getOuterCylindricalR()) for superlayerID in xrange(
                wireTopology.getNSuperLayers())]

    def pick(self, facet):
        return True

    def peel(self, cluster):
        mc_hit_lookup = self.mc_hit_lookup

        superlayer_id = cluster.getISuperLayer()
        size = cluster.size()

        n_background = 0
        total_n_neighbors = 0

        total_drift_length = 0
        total_drift_length_squared = 0

        total_inner_distance = 0
        total_inner_distance_squared = 0

        for wireHit in cluster.items():
            cdcHit = wireHit.getHit()
            if mc_hit_lookup.isBackground(cdcHit):
                n_background += 1

            # Clusterizer writes the number of neighbors into the cell weight
            n_neighbors = wireHit.getAutomatonCell().getCellWeight()
            total_n_neighbors += n_neighbors

            # hit position information
            total_inner_distance += wireHit.getRefPos2D().norm()
            total_inner_distance_squared += wireHit.getRefPos2D().norm() ** 2

            # Drift circle information
            total_drift_length += wireHit.getRefDriftLength()
            total_drift_length_squared += wireHit.getRefDriftLength() ** 2

        if size > 1:
            variance_drift_length = np.sqrt(1.0 / (size - 1) * (total_drift_length_squared - 1.0 * total_drift_length ** 2 / size))
        else:
            variance_drift_length = -1

        is_stereo = superlayer_id % 2 == 1

        distance_to_superlayer_center = self.superlayer_centers[superlayer_id] - 1.0 * total_inner_distance / size

        return dict(
            superlayer_id=superlayer_id,
            size=size,
            n_background=n_background,
            background_fraction=1.0 * n_background / size,
            total_n_neighbors=total_n_neighbors,
            avg_n_neignbors=1.0 * total_n_neighbors / size,
            total_drift_length=total_drift_length,
            mean_drift_length=total_drift_length / size,
            variance_drift_length=variance_drift_length,
            total_inner_distance=total_inner_distance,
            mean_inner_distance=1.0 * total_inner_distance / size,
            is_stereo=is_stereo,
            distance_to_superlayer_center=distance_to_superlayer_center
        )

    save_tree = refiners.save_tree(folder_name="tree")


class WeightCreator():

    def load_plugins(self, name):
        base = "TMVA@@MethodBase"
        regexp1 = ".*_" + str(name) + ".*"
        regexp2 = ".*" + str(name) + ".*"
        class_name = "TMVA::Method" + str(name)
        plugin_name = "TMVA" + str(name)
        ctor1 = "Method" + str(name) + "(DataSetInfo&,TString)"
        ctor2 = "Method" + str(name) + "(TString&,TString&,DataSetInfo&,TString&)"
        ROOT.gPluginMgr.AddHandler(base, regexp1, class_name, plugin_name, ctor1)
        ROOT.gPluginMgr.AddHandler(base, regexp2, class_name, plugin_name, ctor2)

    def __init__(self):
        self.load_plugins("FastBDT")
        self.output_file_name = "weights.root"
        self.input_file_name = None

    def train(self, tree_name, factory_name, cut_signal, excluded_columns):
        input_file = ROOT.TFile(self.input_file_name, "READ")
        output_file = ROOT.TFile(self.output_file_name, "RECREATE")
        input_tree = input_file.Get(tree_name)

        factory = ROOT.TMVA.Factory(factory_name, output_file, "!V:!Silent:Transformations=I")
        factory.AddSignalTree(input_tree)
        factory.AddBackgroundTree(input_tree)

        column_names = [leave.GetName() for leave in input_tree.GetListOfLeaves()]

        for column in column_names:
            if column not in excluded_columns:
                factory.AddVariable(column)

        cut_background = "!(" + str(cut_signal) + ")"

        factory.PrepareTrainingAndTestTree(ROOT.TCut(cut_signal), ROOT.TCut(cut_background), "SplitMode=Random")
        factory.BookMethod(
            ROOT.TMVA.Types.kPlugins,
            "FastBDT",
            "!H:!V:NTrees=100:Shrinkage=0.15:RandRatio=0.5:NCutLevel=8:IgnoreNegWeightsInTraining")

        factory.TrainAllMethods()
        factory.TestAllMethods()
        factory.EvaluateAllMethods()

        output_file.Close()


def create_events():
    run = ClusterFilterValidationRun()
    # you need to provide background files
    run.bkg_files = "/usr/users/frost/bkg.new"
    run.configure_and_execute_from_commandline()
    print basf2.statistics


def create_weights():
    weight_creator = WeightCreator()
    weight_creator.input_file_name = ClusterFilterValidationRun.output_file_name
    weight_creator.train(tree_name="tree/ClusterFilterValidationModule_tree",
                         factory_name="BackgroundHitFinder",
                         cut_signal="background_fraction<=0.8",
                         excluded_columns=["n_background", "background_fraction"])


def plot():

    data = root_pandas.read_root("weights.root", tree_key="TrainTree")
    grouped = data.groupby("classID")
    grouped = DataFrame({"signal": grouped.get_group(0), "background": grouped.get_group(1)})
    print data

    exit()

    gs = GridSpec(4, 3)
    gs.update(wspace=0.5, hspace=0.8)
    index = 0

    for column in data.columns:
        ax = plt.subplot(gs[index])
        if column not in ["classID", "className", "weight"]:
            max_data = np.r_[signal[column], background[column]].max()
            bins = np.linspace(0, max_data, max_data + 1)
            plt.hist(signal[column].values, bins, alpha=0.5)
            plt.hist(background[column].values, bins, alpha=0.5)
            plt.yticks([])
            plt.ylabel("Number", fontsize=8)
            plt.title(column)
            plt.tick_params(labelsize=8)
            index = index + 1

    plt.savefig("all.pdf")


if __name__ == "__main__":
    # create_events()
    # create_weights()
    plot()
