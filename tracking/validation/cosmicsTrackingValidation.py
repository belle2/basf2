#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <contact>oliver.frost@desy.de</contact>
  <output>CosmicsTrackingValidation.root</output>
  <description>This module validates that track finding is capable of reconstructing tracks in cosmics run.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'CosmicsTrackingValidation.root'
CONTACT = 'oliver.frost@desy.de'
N_EVENTS = 10000
ACTIVE = True

import basf2
basf2.set_random_seed(1337)

import argparse

from tracking.validation.run import TrackingValidationRun

import ROOT


class Cosmics(TrackingValidationRun):

    n_events = N_EVENTS
    generator_module = 'Cosmics'
    components = ['CDC', 'MagneticFieldConstant4LimitedRCDC']
    finder_module = 'CDCLocalTracking'
    fit_geometry = None
    pulls = True
    contact = CONTACT
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    argument_parser = argparse.ArgumentParser()

    # Indication if tracks should be fitted.
    # Currently tracks are not fitted because of a segmentation fault related TGeo / an assertation error in Geant4 geometry.
    argument_parser.add_argument('-f', '--fit', action='store_true',
                                 help='Perform fitting of the generated tracks with Genfit. Default is not to perform a fit but use the seed values generated in track finding.'
                                 )

    # Geometry name to be used in the Genfit extrapolation.
    argument_parser.add_argument('-g', '--geometry', choices=['TGeo', 'Geant4'], default='Geant4',
                                 help='Geometry to be used with Genfit.')

    argument_parser.add_argument('-s', '--show', action='store_true',
                                 help='Show generated plots in a TBrowser immediatly.'
                                 )

    arguments = argument_parser.parse_args()

    # Setup the validation run #
    ############################
    cosmics_validation_run = Cosmics()
    if arguments.fit:
        cosmics_validation_run.fit_geometry = arguments.geometry
    else:
        cosmics_validation_run.fit_geometry = None

    # Execute the validation run #
    ##############################
    cosmics_validation_run.execute()

    # Show plots #
    ##############
    if arguments.show:
        output_file_name = cosmics_validation_run.output_file_name
        tFile = ROOT.TFile(output_file_name)
        tBrowser = ROOT.TBrowser()
        tBrowser.BrowseObject(tFile)
        tBrowser.Show()
        raw_input('Press enter to close.')
        tFile.Close()


if __name__ == '__main__':
    if ACTIVE:
        main()
