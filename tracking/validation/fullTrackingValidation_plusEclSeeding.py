#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <contact>simon.kurz@desy.de</contact>
  <input>EvtGenSimNoBkg.root</input>
  <output>fullTrackingValidation_plusECL.root</output>
  <description>This module validates the full track reconstruction including the ECL seeding.</description>
</header>
"""

from tracking.validation.run import TrackingValidationRun
import tracking
import logging
import reconstruction
import basf2
VALIDATION_OUTPUT_FILE = 'fullTrackingValidation_plusECL.root'
N_EVENTS = 1000
ACTIVE = False


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

        reconstruction.add_prefilter_pretracking_reconstruction(path, components=components)

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
    #: do not create expert-level output histograms
    use_expert_folder = False
    #: Include pulls in the validation output
    pulls = True
    #: Include resolution information in the validation output
    resolution = True
    #: name of the output ROOT file
    output_file_name = VALIDATION_OUTPUT_FILE
    #: define empty list of non expert parameters so that no shifter plots are created (to revert just remove following line)
    non_expert_parameters = []


def main():
    basf2.set_random_seed(1337)
    validation_run = fullTrackingValidation_plusECL()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)

    if ACTIVE:
        main()
    else:
        print("This validation deactivated and thus basf2 is not executed.\n"
              "If you want to run this validation, please set the 'ACTIVE' flag above to 'True'.\n"
              "Exiting.")
