#!/usr/bin/env python3
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

hypot = math.hypot


def hypot3(x, y, z):
    return hypot(hypot(x, y), z)


class FacetCreationValidationRun(BrowseTFileOnTerminateRunMixin, StandardEventGenerationRun):
    segment_finder_module = basf2.register_module("SegmentFinderCDCFacetAutomaton")
    segment_finder_module.param({
        "WriteFacets": True,
        "FacetFilter": "fitless_hard",
        "FacetRelationFilter": "none",
    })

    py_profile = True
    output_file_name = "FacetCreationValidation.root"  # Specification for BrowseTFileOnTerminateRunMixin

    def create_argument_parser(self, **kwds):
        argument_parser = super(FacetCreationValidationRun, self).create_argument_parser(**kwds)
        return argument_parser

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(FacetCreationValidationRun, self).create_path()

        segment_finder_module = self.get_basf2_module(self.segment_finder_module)
        main_path.add_module(segment_finder_module)

        # main_path.add_module(AxialStereoPairFitterModule())
        validation_module = FacetCreationValidationModule(output_file_name=self.output_file_name)
        if self.py_profile:
            main_path.add_module(metamodules.PyProfilingModule(validation_module))
        else:
            main_path.add_module(validation_module)

        return main_path


class FacetCreationValidationModule(harvesting.HarvestingModule):

    """Module to collect information about the facet creation cuts and compose validation plots on terminate."""

    cos_cut_value = 0.980

    def __init__(self, output_file_name):
        super(FacetCreationValidationModule, self).__init__(foreach="CDCFacetVector",
                                                            output_file_name=output_file_name)
        self.mc_facet_filter = None

    def initialize(self):
        super(FacetCreationValidationModule, self).initialize()
        self.mc_facet_filter = Belle2.TrackFindingCDC.MCFacetFilter()
        self.real_facet_filter = Belle2.TrackFindingCDC.SimpleFacetFilter()
        self.real_feasible_rl_facet_filter = Belle2.TrackFindingCDC.FeasibleRLFacetFilter(True)

    def prepare(self):
        Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance().fill()

    def pick(self, facet):
        return True

    def peel(self, facet):
        crops = self.peel_target(facet)
        crops.update(self.peel_fit(facet))

        crops.update(self.real_select(facet))

        return crops

    def peel_target(self, facet):
        mc_weight = self.mc_facet_filter(facet)
        mc_decision = np.isfinite(mc_weight)  # Filters for nan

        return dict(
            mc_weight=mc_weight,
            mc_decision=mc_decision,
        )

    def peel_fit(self, facet):
        # No fitless variables for now
        fitless_crops = self.peel_fitless(facet)

        # Fit the tangent lines
        self.fit(facet)

        start_rl_wirehit = facet.getStartRLWireHit()
        start_drift_length_var = start_rl_wirehit.getRefDriftLengthVariance()
        start_drift_length_std = math.sqrt(start_drift_length_var)

        middle_rl_wirehit = facet.getMiddleRLWireHit()
        middle_drift_length_var = middle_rl_wirehit.getRefDriftLengthVariance()
        middle_drift_length_std = math.sqrt(middle_drift_length_var)

        end_rl_wirehit = facet.getEndRLWireHit()
        end_drift_length_var = end_rl_wirehit.getRefDriftLengthVariance()
        end_drift_length_std = math.sqrt(end_drift_length_var)

        start_to_middle_line = facet.getStartToMiddleLine()
        start_to_end_line = facet.getStartToEndLine()
        middle_to_end_line = facet.getMiddleToEndLine()

        start_to_middle_tangential_vector = start_to_middle_line.tangential()
        start_to_end_tangential_vector = start_to_end_line.tangential()
        middle_to_end_tangential_vector = middle_to_end_line.tangential()

        start_to_middle_length = start_to_middle_tangential_vector.norm()
        start_to_end_length = start_to_end_tangential_vector.norm()
        middle_to_end_length = middle_to_end_tangential_vector.norm()

        start_cos = start_to_middle_tangential_vector.cosWith(start_to_end_tangential_vector)
        middle_cos = start_to_middle_tangential_vector.cosWith(middle_to_end_tangential_vector)
        end_cos = start_to_end_tangential_vector.cosWith(middle_to_end_tangential_vector)

        start_phi = math.acos(start_cos)
        middle_phi = math.acos(middle_cos)
        end_phi = math.acos(end_cos)

        # Unexact sigmas up to a common factor equivalent to the simple drift length std
        start_phi_pseudo_sigma = hypot(1.0 / start_to_middle_length, 1.0 / start_to_end_length)
        middle_phi_pseudo_sigma = hypot(1.0 / start_to_middle_length, 1.0 / middle_to_end_length)
        end_phi_pseudo_sigma = hypot(1.0 / start_to_end_length, 1.0 / middle_to_end_length)

        # start_phi_sigma = hypot(start_drift_length_std / start_to_middle_length, start_drift_length_std / start_to_end_length)
        # middle_phi_sigma = hypot(middle_drift_length_std / start_to_middle_length, middle_drift_length_std / middle_to_end_length)
        # end_phi_sigma = hypot(end_drift_length_std / start_to_end_length, end_drift_length_std / middle_to_end_length)

        start_to_middle_sigma_phi = start_drift_length_std / start_to_middle_length
        start_to_end_sigma_phi = start_drift_length_std / start_to_end_length

        middle_to_start_sigma_phi = middle_drift_length_std / start_to_middle_length
        middle_to_end_sigma_phi = middle_drift_length_std / middle_to_end_length

        end_to_start_sigma_phi = end_drift_length_std / start_to_end_length
        end_to_middle_sigma_phi = end_drift_length_std / middle_to_end_length

        start_phi_sigma = hypot3(start_to_end_sigma_phi - start_to_middle_sigma_phi,
                                 middle_to_start_sigma_phi,
                                 end_to_start_sigma_phi)

        middle_phi_sigma = hypot3(start_to_middle_sigma_phi,
                                  middle_to_start_sigma_phi + middle_to_end_sigma_phi,
                                  end_to_middle_sigma_phi)

        end_phi_sigma = hypot3(start_to_end_sigma_phi,
                               middle_to_end_sigma_phi,
                               end_to_start_sigma_phi - end_to_middle_sigma_phi)

        start_phi_pseudo_pull = start_phi / start_phi_pseudo_sigma
        middle_phi_pseudo_pull = start_phi / middle_phi_pseudo_sigma
        end_phi_pseudo_pull = start_phi / end_phi_pseudo_sigma

        start_phi_pull = start_phi / start_phi_sigma
        middle_phi_pull = start_phi / middle_phi_sigma
        end_phi_pull = start_phi / end_phi_sigma

        select_pull = start_phi_pull < 20 and middle_phi_pull < 12.5 and end_phi_pull < 20
        select_pseudo_pull = start_phi_pseudo_pull < 0.3 and middle_phi_pseudo_pull < 0.2 and end_phi_pseudo_pull < 0.3

        crops = dict(
            start_cos=start_cos,
            middle_cos=middle_cos,
            end_cos=end_cos,
            max_cos=max(start_cos, middle_cos, end_cos),
            min_cos=min(start_cos, middle_cos, end_cos),
            no_middle_max_cos=max(start_cos, end_cos),
            no_middle_min_cos=min(start_cos, end_cos),
            start_phi=start_phi,
            midlde_phi=middle_phi,
            end_phi=end_phi,
            start_phi_pseudo_pull=start_phi_pseudo_pull,
            middle_phi_pseudo_pull=middle_phi_pseudo_pull,
            end_phi_pseudo_pull=end_phi_pseudo_pull,
            start_phi_pull=start_phi_pull,
            middle_phi_pull=middle_phi_pull,
            end_phi_pull=end_phi_pull,
            select_pull=select_pull,
            select_pseudo_pull=select_pseudo_pull,
        )
        crops.update(fitless_crops)

        select = self.select(crops)
        crops["select"] = select

        return crops

    def peel_fitless(self, facet):
        shape = facet.getShape()
        start_rl_info = facet.getStartRLInfo()
        middle_rl_info = facet.getMiddleRLInfo()
        end_rl_info = facet.getEndRLInfo()
        superlayer_id = facet.getISuperLayer()

        if shape == facet.c_OrthoCW or shape == facet.c_OrthoCCW:
            # Middle hit must be on the other side of the track
            # Track must at least cross ones between two hits
            select_fitless = (middle_rl_info * shape > 0 and
                              (middle_rl_info != start_rl_info or middle_rl_info != end_rl_info))
            select_fitless_hard = select_fitless

        elif shape == facet.c_MetaCW or shape == facet.c_MetaCCW:
            select_fitless_hard = (middle_rl_info * shape > 0 and
                                   (middle_rl_info != start_rl_info or middle_rl_info != end_rl_info))

            select_fitless = (select_fitless_hard or
                              (middle_rl_info == start_rl_info and middle_rl_info == end_rl_info))

        elif shape == facet.c_Para:
            # Track must not cross between the hits twice
            select_fitless = (start_rl_info == middle_rl_info) or (middle_rl_info == end_rl_info)
            select_fitless_hard = select_fitless

        else:
            # c_Invalid
            select_fitless = False
            select_fitless_hard = False

        return dict(
            superlayer_id=superlayer_id,
            shape=shape,
            abs_shape=abs(shape),
            start_rl_info=start_rl_info,
            middle_rl_info=middle_rl_info,
            end_rl_info=end_rl_info,
            select_fitless=select_fitless,
            select_fitless_hard=select_fitless_hard,
        )

    def fit(self, facet):
        facet.adjustFitLine()

    def select_fitless(self, fitless_crops):
        return crops["select_fitless_hard"]

    def select(self, crops):
        return (
            crops["select_fitless_hard"] and
            crops["no_middle_max_cos"] > 0.994 and
            crops["no_middle_min_cos"] > 0.980 and
            crops["middle_cos"] > 0.980
        )
        # return crops["no_middle_min_cos"] > self.cos_cut_value and crops["select_fitless_hard"]

    def real_select(self, facet):
        real_weigth = self.real_facet_filter(facet)
        real_weigth_fitless = self.real_feasible_rl_facet_filter(facet)
        return dict(
            real_select=np.isfinite(real_weigth),
            real_select_fitless=np.isfinite(real_weigth_fitless),
        )

    # Refiners to be executed at the end of the harvesting / termination of the module
    save_histograms = refiners.save_histograms(outlier_z_score=5.0, allow_discrete=True, folder_name="histograms")
    save_tree = refiners.save_tree(folder_name="tree")

    # Analyse the python selection quality on Monte Carlo
    save_select_quality = refiners.save_classification_analysis(
        folder_name="quality_select",
        truth_name="mc_decision",
        estimate_name="select",
    )

    # Analyse the python pseudo pull selection quality on Monte Carlo
    save_select_cut_quality = refiners.save_classification_analysis(
        folder_name="quality_cuts",
        truth_name="mc_decision",
        estimate_name=["select_pseudo_pull", "select_pull", ]
    )

    # Compare the python selection with the real c++ filter
    save_compare_select = refiners.save_classification_analysis(
        folder_name="compare_real",
        truth_name="real_select",
        estimate_name="select"
    )

    # Fitless preselection
    # Validate that the c++ implementation is in sync with the python one
    save_compare_select_fitless = refiners.save_classification_analysis(
        folder_name="compare_real_fitless",
        truth_name="real_select_fitless",
        estimate_name=["select_fitless", "select_fitless_hard"]
    )

    # Analyse the python fitless selection quality on Monte Carlo
    save_select_fitless_quality = refiners.save_classification_analysis(
        folder_name="quality_fitless",
        truth_name="mc_decision",
        estimate_name=["select_fitless", "select_fitless_hard", ]
    )

    # Classification curves #
    #########################
    save_select_phi_pull = refiners.save_classification_analysis(
        groupby=[None, "abs_shape", 'superlayer_id', ],
        folder_name="quality_phi_pull/{groupby_addition}",
        truth_name="mc_decision",
        estimate_name=["start_phi_pull", "middle_phi_pull", "end_phi_pull", ],
        cut_direction=1,
        cut=20,
        upper_bound=30,
    )

    save_select_phi_pseudo_pull = refiners.save_classification_analysis(
        groupby=[None, "abs_shape", 'superlayer_id', ],
        folder_name="quality_phi_pseudo_pull/{groupby_addition}",
        truth_name="mc_decision",
        estimate_name=["start_phi_pseudo_pull", "middle_phi_pseudo_pull", "end_phi_pseudo_pull", ],
        cut_direction=1,
        cut=0.3,
        upper_bound=0.5,
    )

    save_selection_variables_histograms = refiners.save_classification_analysis(
        groupby=[None, "abs_shape"],
        folder_name="quality_cos/{groupby_addition}",
        estimate_name=[
            "start_cos",
            "middle_cos",
            "end_cos",
            "max_cos",
            "min_cos",
            "no_middle_max_cos",
            "no_middle_min_cos",
        ],
        truth_name="mc_decision",
        cut_direction=-1,  # cut low values
        cut=0.980,
        lower_bound=0.980,
        upper_bound=1,
    )

    @refiners.context(filter_on="mc_decision", folder_name="scatter")
    def save_scatter_cos(self, crops, tdirectory, **kwds):
        max_versus_min_no_middle = ValidationPlot("max_versus_min_no_middle")
        max_versus_min_no_middle.scatter(
            crops["no_middle_max_cos"], crops["no_middle_min_cos"],
            lower_bound=(0.99, 0.90),
        )
        max_versus_min_no_middle.write(tdirectory)

        max_versus_middle = ValidationPlot("max_versus_middle")
        max_versus_middle.scatter(
            crops["no_middle_max_cos"], crops["middle_cos"],
            lower_bound=(0.99, 0.90),
        )
        max_versus_middle.write(tdirectory)

        min_versus_middle = ValidationPlot("min_versus_middle")
        min_versus_middle.scatter(
            crops["no_middle_min_cos"], crops["middle_cos"],
            lower_bound=(0.90, 0.90),
        )
        min_versus_middle.write(tdirectory)

    @refiners.context()
    def print_signal_number(self, crops, tdirectory, **kwds):
        info = get_logger().info

        mc_decisions = crops["mc_decision"]
        n = len(mc_decisions)
        n_signal = np.sum(mc_decisions)
        n_background = n - n_signal
        info("#Signal : %s", n_signal)
        info("#Background : %s", n_background)

        selects = np.nonzero(crops["select"])
        info("#Signal after selection : %s", np.sum(mc_decisions[selects]))
        info("#Background after selection : %s", np.sum(1 - mc_decisions[selects]))


def main():
    run = FacetCreationValidationRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
