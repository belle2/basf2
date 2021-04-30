#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>simon.kurz@desy.de</contact>
  <input>EvtGenSimNoBkg.root</input>
  <output>toCDCfromEclCKFTrackingValidation.root</output>
  <description>This module validates the full track reconstruction including the ECL seeding.</description>
</header>
"""

from tracking.validation.run import TrackingValidationRun
import tracking
import logging
import reconstruction
import basf2
VALIDATION_OUTPUT_FILE = 'fullTrackingValidation_plusECL.root'
N_EVENTS = 10000
ACTIVE = True

basf2.set_random_seed(1337)


class fullTrackingValidation_plusECL(TrackingValidationRun):
    """Validate the full track-finding chain includung the ecl seeded ckf"""
    #: number of events to generate
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    #: no background overlay
    root_input_file = '../EvtGenSimNoBkg.root'

    @staticmethod
    def finder_module(path):
        """Do the standard tracking reconstruction chain"""
        #: ECL has to be added to the list of components for ECL seeding
        components = ["SVD", "CDC", "ECL"]

        reconstruction.add_pretracking_reconstruction(path, components=components)

        tracking.add_tracking_reconstruction(path, components=components,
                                             use_svd_to_cdc_ckf=True, use_ecl_to_cdc_ckf=True)

    #: Define the user parameters for the track-finding module
    tracking_coverage = {
        'WhichParticles': [],  # Include all particles, also secondaries
        'UsePXDHits': False,
        'UseSVDHits': True,
        'UseCDCHits': True,
        "UseReassignedHits": True,
        'UseNLoops': 1
    }

    #: tracks will be already fitted by the modules above
    fit_tracks = False
    #: But we need to tell the validation module to use the fit information
    use_fit_information = True
    #: Include pulls in the validation output
    pulls = True
    #: Include resolution information in the validation output
    resolution = True
    #: name of the output ROOT file
    output_file_name = VALIDATION_OUTPUT_FILE
    #: Store additional information in output file (like the full trees)
    extended = True


def main():
    validation_run = fullTrackingValidation_plusECL()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)

    if ACTIVE:
        main()
