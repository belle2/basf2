#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>oliver.frost@desy.de</contact>
  <input>CosmicsSimNoBkg.root</input>
  <output>CDCCRTrackingValidation.root</output>
  <description>This module validates that track finding is capable of reconstructing tracks in cosmics run.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'CDCCRTrackingValidation.root'
CONTACT = 'oliver.frost@desy.de'
N_EVENTS = 10000
ACTIVE = True

import basf2
basf2.set_random_seed(1337)

import logging

import tracking
from tracking.adjustments import adjust_module
from tracking.validation.run import TrackingValidationRun


class CDCCR(TrackingValidationRun):
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'Cosmics'
    root_input_file = '../CosmicsSimNoBkg.root'
    components = None

    def finder_module(self, path):
        tracking.add_cdc_cr_track_finding(path)

        path.add_module('DAFRecoFitter',
                        pdgCodesToUseForFitting=[13],
                        )

        path.add_module('TrackCreator',
                        pdgCodes=[13],
                        useClosestHitToIP=True,
                        )

    tracking_coverage = {
        'WhichParticles': ['CDC'],  # Include all particles seen in CDC, also secondaries
        'UsePXDHits': False,
        'UseSVDHits': False,
        'UseCDCHits': True,
        'UseOnlyAxialCDCHits': False,
        "UseReassignedHits": True,
    }

    fit_tracks = True
    pulls = True
    resolution = True
    contact = CONTACT
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = CDCCR()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
