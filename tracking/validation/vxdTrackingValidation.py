#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <contact>tracking@belle2.kek.jp</contact>
  <output>VXDTrackingValidation.root</output>
  <description>This module validates that the vxd track finding is capable of reconstructing tracks in Y(4S) runs.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'VXDTrackingValidation.root'
N_EVENTS = 1000
ACTIVE = True

import basf2
basf2.set_random_seed(1337)

import logging

from tracking.validation.run import TrackingValidationRun


class VXD(TrackingValidationRun):
    n_events = N_EVENTS
    generator_module = 'EvtGenInput'
    components = ['BeamPipe', 'MagneticFieldConstant4LimitedRSVD', 'PXD', 'SVD'
                  ]
    finder_module = basf2.register_module('VXDTF')
    finder_module.param({})  # what parameters should be set for the vxd track finding, why is it not working by default?
    fit_geometry = None
    pulls = True
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = VXD()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
