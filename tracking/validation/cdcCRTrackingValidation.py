#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
  <input>CosmicsSimNoBkg.root</input>
  <output>CDCCRTrackingValidation.root</output>
  <description>This module validates that track finding is capable of reconstructing tracks in cosmics run.</description>
</header>
"""

from tracking.validation.run import TrackingValidationRun
import tracking
import logging
import basf2
VALIDATION_OUTPUT_FILE = 'CDCCRTrackingValidation.root'
CONTACT = 'software-tracking@belle2.org'
N_EVENTS = 10000
ACTIVE = False

basf2.set_random_seed(1337)


class CDCCR(TrackingValidationRun):
    """Validate the CDC cosmic-ray track-finding"""
    #: number of events to generate
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'Cosmics'
    #: no background overlay
    root_input_file = '../CosmicsSimNoBkg.root'
    #: all detector components in simulation and reconstruction
    components = None

    def finder_module(self, path):
        """Add the CDC cosmic-ray track-finding module to the basf2 path"""
        tracking.add_cr_track_finding(path)

    #: Define the user parameters for the track-finding module
    tracking_coverage = {
        'WhichParticles': ['CDC'],  # Include all particles seen in CDC, also secondaries
        'UsePXDHits': False,
        'UseSVDHits': False,
        'UseCDCHits': True,
        'UseOnlyAxialCDCHits': False,
        "UseReassignedHits": True,
    }

    #: Use the fit information in validation
    use_fit_information = True
    #: Fit the cosmic-ray tracks
    fit_tracks = True
    #: Include pulls in the validation output
    pulls = True
    #: Include resolution information in the validation output
    resolution = True
    #: name of the contact person
    contact = CONTACT
    #: name of the output ROOT file
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = CDCCR()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
