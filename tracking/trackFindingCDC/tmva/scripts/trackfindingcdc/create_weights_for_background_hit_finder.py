#!/usr/bin/env python
# author Nils Braun, 2015
# This script is intended to create the weights needed for the TMVA in the BackgroundHitFinderModule.
# There should be no need to run this script again - only when something unexpected changes.

import basf2

from ROOT import gSystem
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')

import ROOT
from ROOT import Belle2
import numpy as np

import tracking.validation.harvesting as harvesting
import tracking.validation.refiners as refiners

from tracking.run.event_generation import StandardEventGenerationRun

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

    def initialize(self):
        super(ClusterFilterValidationModule, self).initialize()

    def prepare(self):
        self.mc_hit_lookup.fill()

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

    def train(self):
        input_file = ROOT.TFile(self.input_file_name, "READ")
        output_file = ROOT.TFile(self.output_file_name, "RECREATE")
        input_tree = input_file.Get("tree/ClusterFilterValidationModule_tree")

        factory = ROOT.TMVA.Factory("BackgroundHitFinder", output_file, "!V:!Silent:Transformations=I")
        factory.AddSignalTree(input_tree)
        factory.AddBackgroundTree(input_tree)

        column_names = [leave.GetName() for leave in input_tree.GetListOfLeaves()]

        for column in column_names:
            if column not in ["n_background", "background_fraction"]:
                factory.AddVariable(column)

        cutSignal = "background_fraction<=0.8"
        cutBackground = "background_fraction>0.8"

        factory.PrepareTrainingAndTestTree(ROOT.TCut(cutSignal), ROOT.TCut(cutBackground), "SplitMode=Random")
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
    weight_creator.train()


if __name__ == "__main__":
    create_events()
    create_weights()
