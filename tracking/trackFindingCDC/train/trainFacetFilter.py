#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# This is for training a mva classifier for hit triplets
# It performs a tip better than the current default chi2 filter
# However run time is quite a bit slower which is why it wont be used in the standard chain.
# Nevertheless this script can be used to generate variables to consider
# for improvements or cross checks with --task explore

import sys

from tracking.run.event_generation import StandardEventGenerationRun
from trackfindingcdc.run.training import TrainingRunMixin


class FacetFilterTrainingRun(TrainingRunMixin, StandardEventGenerationRun):
    """Run for recording facets encountered at the filter"""
    #: Suggested number for this analysis
    n_events = 100

    #: Suggested generator module
    generator_module = "generic"
    # detector_setup = "TrackingDetector"

    #: Default task set to explore
    task = "explore"

    #: Name of the truth variables
    truth = "truth_positive"

    #: Option whether to reestimate the drift length
    flight_time_reestimation = False

    #: Option whether to use the least square fit to the hit triplet
    facet_least_square_fit = False

    @property
    def identifier(self):
        """Database identifier of the filter being trained"""
        return "trackfindingcdc_FacetFilter.xml"

    def create_argument_parser(self, **kwds):
        """Convert command-line arguments to basf2 argument list"""

        argument_parser = super().create_argument_parser(**kwds)

        argument_parser.add_argument(
            "-fr",
            "--flight-time-reestimation",
            action="store_true",
            dest="flight_time_reestimation",
            help="Switch to reestimate drift length before fitting."
        )

        argument_parser.add_argument(
            "-fl",
            "--facet-least-square-fit",
            action="store_true",
            dest="facet_least_square_fit",
            help="Switch to fit the facet with least square method for the drift length update"
        )

        return argument_parser

    def create_path(self):
        """
        Sets up a path that plays back pregenerated events or generates events
        based on the properties in the base class.
        """

        path = super().create_path()

        #: Post-process events according to the user's desired task (train, eval, explore)
        if self.task == "train":
            var_sets = [
                "mva",
                "filter(truth)",
            ]

        elif self.task == "eval":
            var_sets = [
                "filter(chi2)",
                "filter(realistic)",
                "filter(mva)",
                "filter(truth)",
            ]

        elif self.task == "explore":
            var_sets = [
                "basic",
                "truth",
                "bend",
                "fit",
                "filter(truth)",
                "filter(realistic)",
                "filter(chi2)",
            ]

            #: Signal some variables to select in the classification analysis
            self.variables = [
                "curv",
                "curv_pull",
                "middle_phi_pull",
                "middle_chi2",
                "fit_0_phi0_sigma",
                "chi2_0",
                "chi2_0_per_s",
                "fit_1_phi0_sigma",
                "chi2_1",
                "chi2_1_per_s",
                "fit_phi0_sigma",
                "chi2 chi2_per_s",
                "realistic_accept",
                "chi2_accept",
            ]

        path.add_module("TFCDC_WireHitPreparer",
                        flightTimeEstimation="outwards",
                        UseNLoops=1.0)

        path.add_module("TFCDC_ClusterPreparer")

        path.add_module("TFCDC_SegmentFinderFacetAutomaton",
                        FacetUpdateDriftLength=self.flight_time_reestimation,
                        FacetLeastSquareFit=self.facet_least_square_fit,
                        FacetFilter="unionrecording",
                        FacetFilterParameters={
                            "rootFileName": self.sample_file_name,
                            "varSets": var_sets,
                        },
                        FacetRelationFilter="none")

        return path


def main():
    """Execute the facet recording"""
    run = FacetFilterTrainingRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    import logging
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format="%(levelname)s:%(message)s")
    main()
