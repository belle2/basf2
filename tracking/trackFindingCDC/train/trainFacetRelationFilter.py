#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This is for training a mva classifier for relations between hit triplets
# It performs a tip better than the current default chi2 filter
# However run time is quite a bit slower which is why it wont be used in the standard chain.
# Nevertheless this script can be used to generate variables to consider
# for improvements or cross checks with --task explore

import sys

from tracking.run.event_generation import StandardEventGenerationRun
from trackfindingcdc.run.training import TrainingRunMixin


class FacetRelationFilterTrainingRun(TrainingRunMixin, StandardEventGenerationRun):
    """Run for recording facets encountered at the filter"""
    #: Suggested number for this analysis
    n_events = 1000

    #: Suggested generator module
    generator_module = "generic"
    # detector_setup = "TrackingDetector"

    #: Default task set to explore
    task = "explore"

    #: Name of the truth variables
    truth = "truth_positive"

    @property
    def identifier(self):
        """Database identifier of the filter being trained"""
        return "trackfindingcdc_FacetRelationFilter.xml"

    def create_path(self):
        """Setup the recording path after the simulation"""
        path = super().create_path()

        #: Post-process events according to the user's desired task (train, eval, explore)
        if self.task == "train":
            var_sets = [
                "mva",
                "filter(truth)",
            ]

        elif self.task == "eval":
            var_sets = [
                "basic",
                "filter(chi2)",
                "filter(chi2_old)",
                "filter(simple)",
                "filter(truth)",
            ]

            self.variables = [
                "chi2_weight",
                "chi2_accept",
                "chi2_old_weight",
                "chi2_old_accept",
                "simple_accept",
            ]

            self.groupby = ["", "superlayer_id"]
            self.auxiliaries = [
                "superlayer_id",
            ]

        elif self.task == "explore":
            var_sets = [
                "basic",
                "bend",
                "fit",
                "filter(chi2)",
                "filter(simple)",
                "filter(truth)",
            ]

            #: Signal some variables to select in the classification analysis
            # self.variables = None #all variables
            self.variables = [
                # "delta_phi",
                # "delta_phi_pull",
                # "delta_phi_pull_per_r",
                # "delta_curv",
                # "delta_curv_pull",
                # "delta_curv_pull_per_r",

                # "cos_delta",
                # "from_middle_cos_delta",
                # "to_middle_cos_delta",

                "chi2_0",
                # "chi2_0_per_s",
                # "erf_0",
                # "fit_0_phi0",
                # "fit_0_cos_delta",

                # "chi2_1",
                # "chi2_1_per_s",
                # "fit_1_phi0",
                # "fit_1_cos_delta",

                # "chi2",
                # "chi2_per_s",
                # "fit_phi0",
                # "fit_cos_delta",

                # "phi0_from_sigma",
                # "phi0_to_sigma",

                # "phi0_ref_pull",
                # "phi0_ref_diff",
                # "phi0_ref_sigma",

                # "chi2_comb",
                # "phi0_comb_pull",
                # "phi0_comb_diff",
                # "phi0_comb_sigma",

                # "chi2_kari_unit",
                # "abs_curv_unit",

                # "chi2_kari_l",
                # "abs_curv_l",

                # "chi2_kari_pseudo",
                # "abs_curv_pseudo",

                # "chi2_kari_proper",
                # "abs_curv_proper",
            ]

            #: group output by superlayer_id
            self.groupby = ["", "superlayer_id"]
            #: auxiliary histograms
            self.auxiliaries = [
                "superlayer_id",
            ]

        path.add_module("TFCDC_WireHitPreparer",
                        flightTimeEstimation="outwards",
                        UseNLoops=1.0)

        path.add_module("TFCDC_ClusterPreparer")

        path.add_module("TFCDC_SegmentFinderFacetAutomaton",
                        FacetRelationFilter="unionrecording",
                        FacetRelationFilterParameters={
                            "rootFileName": self.sample_file_name,
                            "varSets": var_sets,
                        })

        return path


def main():
    """Execute the facet relation recording"""
    run = FacetRelationFilterTrainingRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    import logging
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
