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
  <input>VTXEvtGenSimNoBkg.root</input>
  <output>VTXFullTrackingValidationNoBkg.root</output>
  <description>This script validates the full track finding chain in Y(4S) runs.</description>
</header>
"""

from tracking.path_utils import add_cdc_track_finding, add_vtx_track_finding, add_geometry_modules, \
    add_hit_preparation_modules, add_track_fit_and_track_creator, add_mc_matcher
from tracking.validation.run import TrackingValidationRun
import tracking
import logging
from basf2 import set_log_level, LogLevel
import basf2
VALIDATION_OUTPUT_FILE = 'VTXFullTrackingValidationNoBkg.root'
N_EVENTS = 1000
ACTIVE = True

basf2.set_random_seed(1337)
basf2.set_log_level(LogLevel.ERROR)


def setupFinderModule(path):
    tracking.add_tracking_reconstruction(path, useVTX=True, use_vtx_to_cdc_ckf=True)


class VTXFullTrackingValidationNoBkg(TrackingValidationRun):
    """Validate the full track-finding chain"""
    #: number of events to generate
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    #: no background overlay
    root_input_file = '../VTXEvtGenSimNoBkg.root'
    #: use the complete track-reconstruction chain
    finder_module = staticmethod(setupFinderModule)
    #: Define the user parameters for the track-finding module
    tracking_coverage = {
        'WhichParticles': [],  # Include all particles, also secondaries
        'UsePXDHits': False,
        'UseSVDHits': False,
        'UseCDCHits': True,
        'UseVTXHits': True,
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
    #: name of the output ROOT file
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = VTXFullTrackingValidationNoBkg()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
