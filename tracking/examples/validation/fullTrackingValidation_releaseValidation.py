#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
    This script runs the tracking validation on cdst samples like done for the release validation.
    See produceSample_releaseValidation.py (same folder) for additional branches.
"""


from basf2 import set_random_seed, create_path, process, statistics, register_module, conditions, print_path
from tracking.validation.run import TrackingValidationRun
import logging

set_random_seed(1337)


class Full(TrackingValidationRun):
    """
        Validate the full track-finding chain for the release validation
        using the special samples (cdst with additional branches)
    """
    #: number of events to process
    n_events = 10000
    #: Generator that was used in the simulation (-so)
    generator_module = 'generic'
    #: input file (release validation or generated by produceSample_releaseValidation.py)
    root_input_file = './validationSample.root'

    #: All the track finding and track matching done before, so skip this step
    def finder_module(self, path):
        pass

    # track collection to be used
    recoTracksName = 'RecoTracks'
    # tracks will be already fitted by
    fit_tracks = False
    # But we need to tell the validation module to use the fit information
    use_fit_information = True
    # MC track finding and track matching was already performed
    mc_tracking = False
    # What should be stored in the root file?
    extended = True
    pulls = True
    resolution = True
    saveFullTrees = True
    output_file_name = './FullTrackingValidation_validationSample.root'


def main():
    validation_run = Full()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    main()