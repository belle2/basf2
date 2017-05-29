#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>thomas.hauth@kit.edu</contact>
  <output>Selected via command line</output>
  <description>Generic script to run the tracking validation code on-top of a root file containing
  simulated events. This can speed up the development process allows for an easier reproducibility
  in single events.

  Example to run only the cellular automaton track finder in the CDC:

  python tracking/examples/TrackingValidation.py -i <input.root> -o <output.root> -m TrackFinderCDCAutomaton

  Example to run the full reconstruction:

  python tracking/examples/TrackingValidation.py -i <input.root> -o <output.root> -m FullReco

  </description>
</header>
"""

CONTACT = 'thomas.hauth@kit.edu'
# run at most over this amount of events
N_EVENTS = 100
ACTIVE = True

import basf2
basf2.set_random_seed(1337)

import argparse
from tracking.validation.run import TrackingValidationRun
import ROOT


class Standalone(TrackingValidationRun):
    """Run-Class to run the validation on top of a simulated root file. Most parameters can be chosen from command line."""

    #: The number of events
    n_events = N_EVENTS
    #: load all components
    components = ['PXD', 'SVD', 'CDC', 'BeamPipe',
                  'MagneticFieldConstant4LimitedRCDC']
    #: will be selected by command line
    finder_module = None
    #: Calculate also pull variables
    pulls = True
    #: The contact person (Thomas Hauth)
    contact = CONTACT


def main():
    argument_parser = argparse.ArgumentParser()

    # Indication if tracks should be fitted.
    argument_parser.add_argument(
        '-f',
        '--fit',
        action='store_true',
        help='Perform fitting of the generated tracks with Genfit. Default is not '
        'to perform a fit but use the seed values generated in track finding.')

    argument_parser.add_argument('-s', '--show', action='store_true',
                                 help='Show generated plots in a TBrowser immediatly.')

    argument_parser.add_argument('-i', '--input', type=str,
                                 nargs="?", default=None, required=True,
                                 help='Path to the input root file containing the simulated events.')

    argument_parser.add_argument('-o', '--output', type=str,
                                 nargs="?", default="TrackingValidation.root",
                                 help='Name of the output file used to store the validation plots.')

    argument_parser.add_argument(
        '-m',
        '--module',
        type=str,
        choices=[
            'TFCDC_TrackFinderAutomaton',
            'TrackFinderCDC'
            'Reconstruction'],
        default='Reconstruction',
        help='Short name of track finding module or Reconstruction for the full reconstruction chain.')

    arguments = argument_parser.parse_args()

    # Setup the validation run #
    ############################
    standalone_validation_run = Standalone()
    if arguments.fit:
        standalone_validation_run.fit_tracks = True
    else:
        standalone_validation_run.fit_tracks = False

    # setup input root file #
    #########################
    standalone_validation_run.root_input_file = arguments.input
    # disable simulation
    standalone_validation_run.run_simulation = False

    # setup output root file #
    ##########################
    standalone_validation_run.output_file_name = arguments.output

    # setup reco module(s) to execute #
    ###################################
    standalone_validation_run.finder_module = arguments.module

    # Execute the validation run #
    ##############################
    standalone_validation_run.execute()

    # Show plots #
    ##############
    if arguments.show:
        output_file_name = standalone_validation_run.output_file_name
        tFile = ROOT.TFile(output_file_name)
        tBrowser = ROOT.TBrowser()
        tBrowser.BrowseObject(tFile)
        tBrowser.Show()
        input('Press enter to close.')
        tFile.Close()


if __name__ == '__main__':
    if ACTIVE:
        main()
