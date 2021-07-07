#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import logging
from tracking.run.mixins import BrowseTFileOnTerminateRunMixin
from tracking.run.event_generation import StandardEventGenerationRun
import tracking.metamodules as metamodules
import tracking.harvest.refiners as refiners
import tracking.harvest.harvesting as harvesting
from ROOT import Belle2  # make Belle2 namespace available
import sys

import basf2

from ROOT import gSystem
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')


def get_logger():
    return logging.getLogger(__name__)


CONTACT = "oliver.frost@desy.de"


class ClusterFilterValidationRun(BrowseTFileOnTerminateRunMixin, StandardEventGenerationRun):
    """Prepare and execute a basf2 job to read generated events or generate new events then validate the CDC cluster filter"""
    #: basf2 module for CDC cluster preparation
    cluster_preparation_module = basf2.register_module("TFCDC_ClusterPreparer")

    #: create a python profile
    py_profile = True
    #: output ROOT file
    output_file_name = "ClusterFilterValidation.root"  # Specification for BrowseTFileOnTerminateRunMixin

    def create_argument_parser(self, **kwds):
        """Configure the basf2 job script using the translated command-line arguments"""
        argument_parser = super(ClusterFilterValidationRun, self).create_argument_parser(**kwds)
        return argument_parser

    def create_path(self):
        """
        Sets up a path that plays back pregenerated events or generates events
        based on the properties in the base class.
        """
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
        """Constructor"""
        super(ClusterFilterValidationModule, self).__init__(foreach="CDCWireHitClusterVector",
                                                            output_file_name=output_file_name)
        #: reference to the CDCMCHitlookUp singleton
        self.mc_hit_lookup = Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance()
        #: reference to the CDCWireHitClusterVarSet
        self.cluster_varset = Belle2.TrackFindingCDC.CDCWireHitClusterVarSet()

    def initialize(self):
        """Receive signal at the start of event processing"""
        self.cluster_varset.initialize()
        super(ClusterFilterValidationModule, self).initialize()

    def terminate(self):
        """Receive signal at the end of event processing"""
        self.cluster_varset.terminate()
        super(ClusterFilterValidationModule, self).terminate()

    def prepare(self):
        """Fill the MC hit table"""
        self.mc_hit_lookup.fill()

    def pick(self, facet):
        """Always pick, never reject"""
        return True

    def peel(self, cluster):
        """Extract and store CDC hit and cluster information"""

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

    #: Refiners to be executed at the end of the harvesting / termination of the module
    #: Save a tree of all collected variables in a sub folder
    save_tree = refiners.save_tree(
        #: \cond
        folder_name="tree"
        #: \cond
    )
    #: Save histograms in a sub folder
    save_histograms = refiners.save_histograms(
        #: \cond
        outlier_z_score=5.0,
        allow_discrete=True,
        folder_name="histograms"
        #: \cond
    )


def main():
    run = ClusterFilterValidationRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
