#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import os.path
import basf2

from trackfindingcdc.run.recording import RecordingRun
import logging


class CDCFacetRecordingRun(RecordingRun):
    """Run for recording facets encountered at the filter"""
    detector_setup = "TrackingDetectorConstB"
    n_events = 100

    @property
    def root_output_file_name(self):
        print("get")
        file_name = ""
        if self.n_loops == self.n_loops:
            file_name = "loop-" + str(self.n_loops)
        else:
            file_name = "all"

        if self.flight_time_reestimation:
            file_name += "-re"

        if self.facet_least_square_fit:
            file_name += "-lq"

        if self.skim:
            file_name += "-skim-" + self.skim

        if self.root_input_file:
            file_name += "-" + os.path.split(self.root_input_file)[1]
        else:
            file_name += ".root"

        return file_name

    varsets = [
        "basic",
        "truth",
        "bend",
        "fit",
        'filter(truth)',
        'filter(realistic)',
        'filter(chi2)',
    ]

    # varsets = ["tmva", "filter(truth)", ]

    recording_finder_module = basf2.register_module("SegmentFinderCDCFacetAutomaton")
    recording_filter_parameter_name = "FacetFilterParameters"
    recording_finder_module.param({
        "FacetFilter": "unionrecording",
        "FacetRelationFilter": "none"
    })

    flight_time_reestimation = False
    facet_least_square_fit = False

    def create_argument_parser(self, **kwds):
        argument_parser = super().create_argument_parser(**kwds)

        argument_parser.add_argument(
            "-fr",
            "--flight-time-reestimation",
            action="store_true",
            dest="flight_time_reestimation",
            help=("Switch to reestimate drift length before fitting.")
        )

        argument_parser.add_argument(
            "-fl",
            "--facet-least-square-fit",
            action="store_true",
            dest="facet_least_square_fit",
            help=(" Switch to fit the facet with least square method for the drift length update")
        )

        return argument_parser

    def configure(self, arguments):
        super().configure(arguments)

        self.recording_finder_module.param(
            dict(FacetUpdateDriftLength=self.flight_time_reestimation,
                 FacetLeastSquareFit=self.facet_least_square_fit)
        )


def main():
    """Execute the facet recording"""
    run = CDCFacetRecordingRun()
    run.configure_and_execute_from_commandline()


if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')
    main()
