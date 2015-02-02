#!/usr/bin/env python
# -*- coding: utf-8 -*-

import logging

import basf2

from tracking.run.event_generation import ReadOrGenerateEventsRun
from tracking.modules import StandardTrackingReconstructionModule, \
    BrowseFileOnTerminateModule
from tracking.validation.module import TrackingValidationModule
import tracking.utilities as utilities

TRACKING_MAILING_LIST = 'tracking@belle2.kek.jp'


def get_logger():
    return logging.getLogger(__name__)


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


def get_basf2_module_name(module_or_module_name):
    if isinstance(module_or_module_name, str):
        module_name = module_or_module_name
        return module_name
    elif isinstance(module_or_module_name, basf2.Module):
        module = module_or_module_name
        module_name = module.name()
        return module_name
    else:
        message_template = \
            '%s of type %s is neither a module nor the name of module. Expected str or basf2.Module instance.'
        raise ValueError(message_template % (module_or_module_name,
                                             type(module_or_module_name)))


class TrackingValidationRun(ReadOrGenerateEventsRun):

    finder_module = 'StandardReco'  # To be specified
    tracking_coverage = {'UsePXDHits': True, 'UseSVDHits': True,
                         'UseCDCHits': True}
    fit_geometry = None  # Determines which fit geometry should be used. Validate seed values of the candidates if None.
    pulls = True  # Generate the pulls of track parameters
    contact = TRACKING_MAILING_LIST
    output_file_name = 'TrackingValidation.root'
    show_results = False

    # The default way to add the validation module to the path
    # Derived classes can overload this method modify the validation module
    # or add more than one validation steps
    def preparePathValidation(self, main_path):

        # Validation module generating plots
        fit = bool(self.fit_geometry)
        trackingValidationModule = TrackingValidationModule(self.name,
                                                            contact=self.contact, fit=fit, pulls=self.pulls,
                                                            output_file_name=self.output_file_name)
        main_path.add_module(trackingValidationModule)

    def create_argument_parser(self, **kwds):
        argument_parser = super(TrackingValidationRun,
                                self).create_argument_parser(**kwds)

        # Indication if tracks should be fitted with which geomety
        # Currently tracks are not fitted because of a segmentation fault related TGeo / an assertation error in Geant4 geometry.
        # Geometry name to be used in the Genfit extrapolation.
        argument_parser.add_argument(  # Legacy
                                       # Legacy
            '-f',
            '--finder',
            choices=utilities.NonstrictChoices([
                'StandardReco',
                'VXDTF',
                'TrackFinderCDCAutomaton',
                'TrackFinderCDCLegendre',
                'CDCLegendreTracking',
                'CDCLocalTracking',
            ]),
            default=self.finder_module,
            dest='finder_module',
            help='Name of the finder module to be evaluated.',
        )

        argument_parser.add_argument('--fit-geometry', choices=['TGeo',
                                                                'Geant4'], default=self.fit_geometry,
                                     dest='fit_geometry',
                                     help='Geometry to be used with Genfit. If unset validate the seed values instead'
                                     )

        argument_parser.add_argument(
            '-s',
            '--show',
            action='store_true',
            default=self.show_results,
            dest='show_results',
            help='Show generated plots in a TBrowser immediatly.',
        )

        return argument_parser

    def determine_tracking_coverage(self, finder_module_or_name):
        finder_module_name = get_basf2_module_name(finder_module_or_name)
        if finder_module_name == 'CDCLocalTracking' or finder_module_name \
                == 'CDCLegendreTracking' \
                or finder_module_name.startswith('TrackFinderCDC'):
            return {'UsePXDHits': False, 'UseSVDHits': False,
                    'UseCDCHits': True}
        elif finder_module_name == 'VXDTF':
            return {'UsePXDHits': True, 'UseSVDHits': True,
                    'UseCDCHits': False}
        elif finder_module_name in {'StandardReco',
                                    'StandardTrackingReconstruction'}:

            return {'UsePXDHits': 'PXD' in self.components,
                    'UseSVDHits': 'SVD' in self.components,
                    'UseCDCHits': 'CDC' in self.components}
        else:
            get_logger().info('Could not determine tracking coverage for module name %s. Using value stored in self.tracking_coverage which is %s', finder_module_name, self.tracking_coverage)
            return self.tracking_coverage

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(TrackingValidationRun, self).create_path()

        if self.show_results:
            browseFileOnTerminateModule = \
                BrowseFileOnTerminateModule(self.output_file_name)
            main_path.add_module(browseFileOnTerminateModule)

        # Setup track finder
        # determine which sub-detector hits will be used
        tracking_coverage = \
            self.determine_tracking_coverage(self.finder_module)

        if self.finder_module == 'StandardReco':
            trackFinderModule = \
                StandardTrackingReconstructionModule(components=self.components)
            main_path.add_module(trackFinderModule)
        else:
            trackFinderModule = get_basf2_module(self.finder_module)
            main_path.add_module(trackFinderModule)

            # setting up fitting is only necessary when testing
            # track finding comonenst ex-situ
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
        trackFinderMCTruthModule.param(tracking_coverage)
        main_path.add_module(trackFinderMCTruthModule)

        # Track matcher
        mcTrackMatcherModule = basf2.register_module('MCTrackMatcher')
        mcTrackMatcherModule.param({'MCGFTrackCandsColName': 'MCTrackCands',
                                    'MinimalPurity': 0.66,
                                    'RelateClonesToMCParticles': True})
        mcTrackMatcherModule.param(tracking_coverage)
        main_path.add_module(mcTrackMatcherModule)

        # add the validation module to the path
        self.preparePathValidation(main_path)

        return main_path


def main():
    trackingValiddationRun = TrackingValidationRun()
    trackingValiddationRun.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig()
    main()
