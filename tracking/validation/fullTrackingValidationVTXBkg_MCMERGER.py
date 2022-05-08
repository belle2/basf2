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
  <input>VTXEvtGenSim.root</input>
  <output>VTXFullTrackingValidationBkg_MCMERGER.root</output>
  <description>This script validates the full track finding chain in Y(4S) runs with background.</description>
</header>
"""

from tracking.validation.run import TrackingValidationRun
import tracking
import logging
from basf2 import LogLevel
import basf2
VALIDATION_OUTPUT_FILE = 'VTXFullTrackingValidationBkg_MCMERGER.root'
N_EVENTS = 1000
ACTIVE = True

basf2.set_random_seed(1337)
basf2.set_log_level(LogLevel.INFO)


def setupFinderModule(path):
    tracking.add_tracking_reconstruction(path, useVTX=True, use_vtx_to_cdc_ckf=True, use_mc_vtx_cdc_merger=True)


class VTXFullTrackingValidationBkg_MCMERGER(TrackingValidationRun):
    """Validate the full track-finding chain, including background overlay"""
    #: number of events to generate
    n_events = N_EVENTS
    #: Y(4S) events with background overlay
    root_input_file = '../VTXEvtGenSim.root'
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
    #: Switch to use the extended harvesting validation instead
    extended = True
    #: Only works in extended mode
    saveFullTrees = True
    #: do not create expert-level output histograms
    use_expert_folder = True
    #: Include pulls in the validation output
    pulls = True
    #: Include resolution information in the validation output
    resolution = True
    #: name of the output ROOT file
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = VTXFullTrackingValidationBkg_MCMERGER()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
