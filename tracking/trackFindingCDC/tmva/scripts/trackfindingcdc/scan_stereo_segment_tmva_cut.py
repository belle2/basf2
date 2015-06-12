#!/usr/bin/env python

from tracking.run.event_generation import StandardEventGenerationRun
from tracking import modules

import logging
import sys
import numpy as np

from multiprocessing import Pool


class FullRun(StandardEventGenerationRun):
    #: Tester module for all cdc tracking in on path (plus combiners)

    n_events = 1000
    tmva_cut = 0.1

    def create_path(self):
        #: Creates the path with all the modules in a row
        main_path = super(FullRun, self).create_path()

        main_path.add_module(
            modules.CDCFullFinder(
                output_track_cands_store_array_name="TrackCands",
                combiner_tmva_cut=self.tmva_cut))

        main_path.add_module(
            modules.CDCValidation(
                track_candidates_store_array_name="TrackCands",
                output_file_name="output_%.2f.root" % self.tmva_cut))

        return main_path


class ParameterScanner():

    def __init__(self, lambda_function, parameter_ranges):
        self.lambda_function = lambda_function
        self.parameter_ranges = parameter_ranges

    def scan_parameter_space(self):
        import itertools

        every_parameter_combination = itertools.product(*(self.parameter_ranges))
        p = Pool(8)
        p.map(extract_parameters, every_parameter_combination)
        p.close()


def extract_parameters(x):
    return main(*x)


def main(tmva_cut):
    run = FullRun()
    run.tmva_cut = tmva_cut
    run.configure_and_execute_from_commandline()

if __name__ == "__main__":
    #: Main function
    logging.basicConfig(stream=sys.stdout, level=logging.INFO, format='%(levelname)s:%(message)s')

    short_scanner = ParameterScanner(main,
                                     [np.arange(0.4, 1.0, 0.05)])
    short_scanner.scan_parameter_space()
