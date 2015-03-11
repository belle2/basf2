#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import math
import numpy as np

import basf2

from ROOT import gSystem
gSystem.Load('libtracking')
gSystem.Load('libtrackFindingCDC')

from ROOT import Belle2  # make Belle2 namespace available
from ROOT import std

from tracking.utilities import NonstrictChoices
from tracking.validation.utilities import prob, is_primary
from tracking.validation.plot import ValidationPlot

import tracking.validation.harvesting as harvesting
import tracking.validation.refiners as refiners
import tracking.metamodules as metamodules

from tracking.run.event_generation import StandardEventGenerationRun
from tracking.run.mixins import BrowseTFileOnTerminateRunMixin


import logging


def get_logger():
    return logging.getLogger(__name__)


CONTACT = "oliver.frost@desy.de"


class ClusterFilterValidationRun(BrowseTFileOnTerminateRunMixin, StandardEventGenerationRun):
    segment_finder_module = basf2.register_module("SegmentFinderCDCFacetAutomatonDev")
    segment_finder_module.param({
        "WriteClusters": True,
        "FacetFilter": "none",
        "FacetNeighborChooser": "none",
    })

    py_profile = True
    output_file_name = "ClusterFilterValidation.root"  # Specification for BrowseTFileOnTerminateRunMixin

    def create_argument_parser(self, **kwds):
        argument_parser = super(ClusterFilterValidationRun, self).create_argument_parser(**kwds)
        return argument_parser

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

        for wireHit in cluster.items():
            cdcHit = wireHit.getHit()
            if mc_hit_lookup.isBackground(cdcHit):
                n_background += 1

            # Clusterizer writes the number of neighbors into the cell weight
            n_neighbors = wireHit.getAutomatonCell().getCellWeight()
            total_n_neighbors += n_neighbors

            # Drift circle information
            wireHit.getRefPos2D()
            wireHit.getRefDriftLength()
            wireHit.getRefDriftLengthVariance()

        # etc.....

        # When do we actually want to reject a cluster?

        return dict(
            superlayer_id=superlayer_id,
            size=size,
            n_background=n_background,
            background_fraction=1.0 * n_background / size,
            total_n_neighbors=total_n_neighbors,
            avg_n_neignbors=1.0 * total_n_neighbors / size,
        )

    save_tree = refiners.save_tree(folder_name="tree")
    # save_histograms = refiners.save_histograms(outlier_z_score=5.0, allow_discrete=True, folder_name="histograms")


def main():
    run = ClusterFilterValidationRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
