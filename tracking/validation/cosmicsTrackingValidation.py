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
  <contact>software-tracking@belle2.org</contact>
  <input>CosmicsSimNoBkgTrackingVolume.root</input>
  <output>CosmicsTrackingValidation.root</output>
  <description>Validation of the default cosmic track finding (VXD+CDC).</description>
</header>
"""

from tracking.validation.run import TrackingValidationRun
from tracking import add_cr_tracking_reconstruction
import logging
import basf2
VALIDATION_OUTPUT_FILE = 'CosmicsTrackingValidation.root'
CONTACT = 'software-tracking@belle2.org'
N_EVENTS = 10000
ACTIVE = True


class Cosmics(TrackingValidationRun):
    """
    derived class that sets up the cosmics validation
    """
    #: number of events to be prcessed
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'Cosmics'
    #: the input file on which the validation is performed
    root_input_file = '../CosmicsSimNoBkgTrackingVolume.root'
    #: use full detector for validation
    components = None
    #: finder module: sets up the track finder used in this validation
    finder_module = staticmethod(add_cr_tracking_reconstruction)
    #: set up the MC track finder
    tracking_coverage = {
        'WhichParticles': ['CDC'],  # Include all particles seen in CDC, also secondaries (dont put SVD, PXD here)
        'UsePXDHits': True,
        'UseSVDHits': True,
        'UseCDCHits': True,
        'UseOnlyAxialCDCHits': False,
        "UseReassignedHits": True,
        'UseNLoops': 1
    }
    #: if true an analysis of the pull distribution will be performed
    pulls = True
    #: the responsible contact in case the validation plots show issue
    contact = CONTACT
    #: name of the output file
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    basf2.set_random_seed(1337)
    validation_run = Cosmics()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
    else:
        print("This validation deactivated and thus basf2 is not executed.\n"
              "If you want to run this validation, please set the 'ACTIVE' flag above to 'True'.\n"
              "Exiting.")
