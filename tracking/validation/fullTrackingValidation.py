#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>software-tracking@belle2.org</contact>
  <input>EvtGenSimNoBkg.root</input>
  <output>FullTrackingValidation.root</output>
  <description>This script validates the full track finding chain in Y(4S) runs.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'FullTrackingValidationTest_with_CDCQI_e1003_r2_CDC08_cut_deleteTracksTrue_resetFlagTrue.root'
N_EVENTS = 1000
ACTIVE = True

import basf2
basf2.set_random_seed(1337)

import logging
import tracking

from tracking.validation.run import TrackingValidationRun


class Full(TrackingValidationRun):
    """Validate the full track-finding chain"""
    #: number of events to generate
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    #: no background overlay
    root_input_file = '../EvtGenSim_1k_exp1003_run2.root'
    #: use the complete track-reconstruction chain
    #: finder_module = staticmethod(tracking.add_tracking_reconstruction)

    def finder_module(self, path):
        tracking.add_tracking_reconstruction(path, add_cdcTrack_QI=True, add_vxdTrack_QI=False, add_recoTrack_QI=False)
    #: Define the user parameters for the track-finding module
    tracking_coverage = {
        'WhichParticles': [],  # Include all particles, also secondaries
        'UsePXDHits': True,
        'UseSVDHits': True,
        'UseCDCHits': True,
        "UseReassignedHits": True,
        'UseNLoops': 1
    }
    #: tracks will be already fitted by
    #: add_tracking_reconstruction finder module set above
    fit_tracks = False
    #: But we need to tell the validation module to use the fit information
    use_fit_information = True
    #: do not create expert-level output histograms
    use_expert_folder = False
    #: Include pulls in the validation output
    pulls = True
    #: Include resolution information in the validation output
    resolution = True
    # save trees such that manual studies are possible
    extended = True
    saveFullTrees = True
    #: name of the output ROOT file
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = Full()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
