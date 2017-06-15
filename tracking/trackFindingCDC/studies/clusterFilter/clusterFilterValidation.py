#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import math
import numpy as np

import basf2

from ROOT import gSystem
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')

from ROOT import Belle2  # make Belle2 namespace available
from ROOT import std

from tracking.utilities import NonstrictChoices
from tracking.validation.utilities import prob, is_primary
from tracking.validation.plot import ValidationPlot

import tracking.harvest.harvesting as harvesting
import tracking.harvest.refiners as refiners
import tracking.metamodules as metamodules

from tracking.run.event_generation import StandardEventGenerationRun
from tracking.run.mixins import BrowseTFileOnTerminateRunMixin


import logging


def get_logger():
    return logging.getLogger(__name__)


CONTACT = "oliver.frost@desy.de"


class ClusterFilterValidationRun(BrowseTFileOnTerminateRunMixin, StandardEventGenerationRun):
    cluster_preparation_module = basf2.register_module("TFCDC_ClusterPreparer")

    py_profile = True
    output_file_name = "ClusterFilterValidation.root"  # Specification for BrowseTFileOnTerminateRunMixin

    def create_argument_parser(self, **kwds):
        argument_parser = super(ClusterFilterValidationRun, self).create_argument_parser(**kwds)
        return argument_parser

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(ClusterFilterValidationRun, self).create_path()

        cluster_preparation_module = self.get_basf2_module(self.cluster_preparation_module)
        main_path.add_module(cluster_preparation_module)

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
        self.cluster_varset = Belle2.TrackFindingCDC.CDCWireHitClusterVarSet()

    def initialize(self):
        self.cluster_varset.initialize()
        super(ClusterFilterValidationModule, self).initialize()

    def terminate(self):
        self.cluster_varset.terminate()
        super(ClusterFilterValidationModule, self).terminate()

    def prepare(self):
        self.mc_hit_lookup.fill()

    def pick(self, facet):
        return True

    def peel(self, cluster):
        mc_hit_lookup = self.mc_hit_lookup

        self.cluster_varset.extract(cluster)
        cluster_crops = self.cluster_varset.getNamedValues()
        cluster_crops = dict(cluster_crops)

        # Truth variables
        n_background = 0
        for wireHit in list(cluster.items()):
            cdcHit = wireHit.getHit()
            if mc_hit_lookup.isBackground(cdcHit):
                n_background += 1

        truth_dict = dict(
            n_background_truth=n_background,
            background_fraction_truth=1.0 * n_background / cluster.size()
        )

        cluster_crops.update(truth_dict)
        return cluster_crops

    save_tree = refiners.save_tree(folder_name="tree")
    save_histograms = refiners.save_histograms(outlier_z_score=5.0,
                                               allow_discrete=True,
                                               folder_name="histograms")


def main():
    run = ClusterFilterValidationRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
