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


class CDCFull(TrackingValidationRun):
    n_events = N_EVENTS
    root_input_file = '../EvtGenSimNoBkg.root'
    finder_module = staticmethod(tracking.add_cdc_track_finding)
    tracking_coverage = {
        'UsePXDHits': False,
        'UseSVDHits': False,
        'UseCDCHits': True,
        'UseOnlyAxialCDCHits': False,
        # 'WhichParticles': ['CDC'], # Uncomment to count also secondary tracks
        # 'EnergyCut': 0.1,
    }
    fit_geometry = None
    pulls = True
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = CDCFull()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
