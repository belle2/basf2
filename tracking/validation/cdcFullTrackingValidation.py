#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>Thomas.Hauth@kit.edu</contact>
  <input>EvtGenSimNoBkg.root</input>
  <output>CDCFullTrackingValidation.root</output>
  <description>This script validates the full CDC tracking chain with a legendre step first
  and a cellular automaton step second in Y(4S) runs.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'CDCFullTrackingValidation.root'
N_EVENTS = 1000
ACTIVE = True

import basf2
basf2.set_random_seed(1337)

import logging
import tracking

from tracking.validation.run import TrackingValidationRun
from tracking.adjustments import adjust_module


class CDCFull(TrackingValidationRun):
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    root_input_file = '../EvtGenSimNoBkg.root'

    def finder_module(self, path):
        tracking.add_cdc_track_finding(path)
        # adjust_module(path, "WireHitPreparer",
        #               UseNLoops=1)

    tracking_coverage = {
        'WhichParticles': ['CDC'],  # Include all particles seen in CDC, also secondaries
        'UsePXDHits': False,
        'UseSVDHits': False,
        'UseCDCHits': True,
        'UseOnlyAxialCDCHits': False,
        "UseNLoops": 1,
        "UseOnlyBeforeTOP": True,
        # 'EnergyCut': 0.1,
    }
    pulls = True
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = CDCFull()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
