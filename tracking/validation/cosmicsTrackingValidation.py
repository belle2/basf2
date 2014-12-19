#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <contact>oliver.frost@desy.de</contact>
  <output>cosmicsTrackingValidation.root</output>
  <description>This module validates that track finding is capable of reconstructing tracks in cosmics run.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'CosmicsTrackingValidation.root'
CONTACT = 'oliver.frost@desy.de'
N_EVENTS = 10000

TRACKING_MAILING_LIST = 'tracking@belle2.kek.jp'

import basf2
basf2.set_random_seed(1337)

import argparse

import os
import sys
import logging
import collections

import numpy as np
import math

import simulation

from tracking.validation.module import TrackingValidationModule

import ROOT
from ROOT import Belle2


def main():
    argument_parser = argparse.ArgumentParser()

    # Indication if tracks should be fitted.
    # Currently tracks are not fitted because of a segmentation fault related TGeo / an assertation error in Geant4 geometry.
    argument_parser.add_argument('-f', '--fit', action='store_true',
                                 help='Perform fitting of the generated tracks with Genfit. Default is not to perform a fit but use the seed values generated in track finding.'
                                 )

    # Geometry name to be used in the Genfit extrapolation.
    argument_parser.add_argument('-g', '--geometry', choices=['TGeo', 'Geant4'
                                 ], default='Geant4',
                                 help='Geometry to be used with Genfit.')

    argument_parser.add_argument('-s', '--show', action='store_true',
                                 help='Show generated plots in a TBrowser immediatly.'
                                 )

    arguments = argument_parser.parse_args()

    cosmics_validation_run = Cosmics()
    if arguments.fit:
        cosmics_validation_run.fit_geometry = arguments.geometry
    else:
        cosmics_validation_run.fit_geometry = None

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


class TrackingValidationRun(object):

    n_events = 10000
    generator_module = 'EvtGenInput'
    components = ['PXD', 'SVD', 'CDC', 'BeamPipe',
                  'MagneticFieldConstant4LimitedRCDC']
    finder_module = None  # To be specified
    tracking_coverage = {'UsePXDHits': True, 'UseSVDHits': True,
                         'UseCDCHits': True}
    fit_geometry = None
    pulls = True
    contact = TRACKING_MAILING_LIST
    output_file_name = 'StandardTrackingValidation.root'

    # Allow override from instances only in these field names to prevent some spelling errors
    __slots__ = [
        'n_events',
        'generator_module',
        'components',
        'finder_module',
        'tracking_coverage',
        'fit_geometry',
        'contract',
        'output_file_name',
        ]

    @property
    def name(self):
        return self.__class__.__name__

    def execute(self):
        # Compose basf2 module path #
        #############################
        main_path = basf2.create_path()

        # Master module
        eventInfoSetterModule = basf2.register_module('EventInfoSetter')
        eventInfoSetterModule.param({'evtNumList': [self.n_events],
                                    'runList': [1], 'expList': [1]})
        main_path.add_module(eventInfoSetterModule)

        # Progress module
        progressModule = basf2.register_module('Progress')
        main_path.add_module(progressModule)

        # Generator
        generatorModule = get_basf2_module(self.generator_module)
        main_path.add_module(generatorModule)

        # Simulation
        components = self.components
        simulation.add_simulation(main_path, components=components)

        # Setup track finder
        trackFinderModule = get_basf2_module(self.finder_module)
        main_path.add_module(trackFinderModule)

        if self.fit_geometry:
            # Prepare Genfit extrapolation
            setupGenfitExtrapolationModule = \
                basf2.register_module('SetupGenfitExtrapolation')
            setupGenfitExtrapolationModule.param({'whichGeometry': self.fit_geometry})
            main_path.add_module(setupGenfitExtrapolationModule)

            # Fit tracks
            genFitterModule = basf2.register_module('GenFitter')
            genFitterModule.param({'PDGCodes': [13]})
            main_path.add_module(genFitterModule)

        # Reference Monte Carlo tracks
        trackFinderMCTruthModule = basf2.register_module('TrackFinderMCTruth')
        trackFinderMCTruthModule.param({'WhichParticles': ['primary'],
                                       'EnergyCut': 0.1,
                                       'GFTrackCandidatesColName': 'MCTrackCands'
                                       })
        trackFinderMCTruthModule.param(self.tracking_coverage)
        main_path.add_module(trackFinderMCTruthModule)

        # Track matcher
        mcTrackMatcherModule = basf2.register_module('MCTrackMatcher')
        mcTrackMatcherModule.param({'MCGFTrackCandsColName': 'MCTrackCands',
                                   'MinimalPurity': 0.66,
                                   'RelateClonesToMCParticles': True})
        mcTrackMatcherModule.param(self.tracking_coverage)
        main_path.add_module(mcTrackMatcherModule)

        # Validation module generating plots
        fit = bool(self.fit_geometry)
        pulls = self.pulls
        contact = self.contact
        output_file_name = self.output_file_name
        trackingValidationModule = TrackingValidationModule(self.name,
                contact=contact, fit=fit, pulls=pulls,
                output_file_name=output_file_name)
        main_path.add_module(trackingValidationModule)

        # Run basf2 module path #
        #########################
        print 'Start processing'
        basf2.process(main_path)
        print basf2.statistics


def get_basf2_module(module_or_module_name):
    if isinstance(module_or_module_name, str):
        module_name = module_or_module_name
        module = basf2.register_module(module_name)
        return module
    elif isinstance(module_or_module_name, basf2.Module):
        module = module_or_module_name
        return module
    else:
        message_template = \
            '%s of type %s is neither a module nor the name of module. Expected str or basf2.Module instance.'
        raise ValueError(message_template % (module_or_module_name,
                         type(module_or_module_name)))


class Cosmics(TrackingValidationRun):

    n_events = N_EVENTS
    generator_module = 'Cosmics'
    components = ['CDC', 'MagneticFieldConstant4LimitedRCDC']
    finder_module = 'CDCLocalTracking'
    tracking_coverage = {'UsePXDHits': False, 'UseSVDHits': False,
                         'UseCDCHits': True}
    fit_geometry = None
    pulls = True
    contact = CONTACT
    output_file_name = 'CosmicsTrackingValidation.root'


if __name__ == '__main__':
    main()

