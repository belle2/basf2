#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2

from tracking.run.event_generation import ReadOrGenerateEventsRun
from tracking.modules import (
    StandardTrackingReconstructionModule
)

from tracking.metamodules import IfMCParticlesPresentModule

import tracking.utilities as utilities

import logging


def get_logger():
    return logging.getLogger(__name__)


class ReadOrGenerateTrackedEventsRun(ReadOrGenerateEventsRun):
    finder_module = None
    #: States which detectors the finder module covers like as a dictionary like
    #: {'UsePXDHits': True, 'UseSVDHits': True,'UseCDCHits': True, 'UseOnlyAxialCDCHits': False}
    tracking_coverage = {}
    fit_geometry = None  # Determines which fit geometry should be used.
    fit_tracks = False  # If true, tracks will be fitted after they have been found
    # This flag is ignored when running the full reco chain
    trackCandidatesColumnName = 'TrackCands'

    def create_argument_parser(self, **kwds):
        argument_parser = super(ReadOrGenerateTrackedEventsRun, self).create_argument_parser(**kwds)

        # Indication if tracks should be fitted with which geomety
        # Currently tracks are not fitted because of a segmentation fault related TGeo / an assertation error in Geant4 geometry.
        # Geometry name to be used in the Genfit extrapolation.
        argument_parser.add_argument(
            '-f',
            '--finder',
            choices=utilities.NonstrictChoices(['StandardReco',
                                                'VXDTF',
                                                'TrackFinderCDCAutomaton',
                                                'TrackFinderCDCLegendreTracking',
                                                'Trasan',
                                                ]),
            default=self.finder_module,
            dest='finder_module',
            help='Name of the finder module to be evaluated.',)

        argument_parser.add_argument(
            '--fit-geometry',
            choices=['TGeo', 'Geant4', 'default'],
            default=self.fit_geometry,
            dest='fit_geometry',
            help='Geometry to be used with Genfit. If unset no fit is performed'
        )

        argument_parser.add_argument(
            '-so',
            '--simulate-only',
            action='store_true',
            default=self.simulate_only,
            dest='simulate_only',
            help='Only generate and simulate the events, but do not run any tracking or validation code')

        return argument_parser

    @staticmethod
    def get_module_param(module, name):
        parameters = module.available_params()
        for parameter in parameters:
            if name == parameter.name:
                return parameter.values
            else:
                raise AttributeError('%s module does not have a parameter named %s' % (module, name))

    def determine_tracking_coverage(self, finder_module_or_name):
        # Use value overwritten in the concrete subclass
        if self.tracking_coverage:
            return self.tracking_coverage

        # Else try to determine it from the module name
        finder_module_name = self.get_basf2_module_name(finder_module_or_name)

        if (finder_module_name == 'TrackFinderCDCLegendreTracking' or
                finder_module_name.startswith("TrackFinderCDCAxial")):
            return {'UsePXDHits': False,
                    'UseSVDHits': False,
                    'UseCDCHits': True,
                    'UseOnlyAxialCDCHits': True}

        elif (finder_module_name in ('add_cdc_track_finding', 'Trasan') or
                finder_module_name.startswith('TrackFinderCDC')):
            return {'UsePXDHits': False,
                    'UseSVDHits': False,
                    'UseCDCHits': True,
                    'UseOnlyAxialCDCHits': False}

        elif finder_module_name == 'VXDTF':
            return {'UsePXDHits': True,
                    'UseSVDHits': True,
                    'UseCDCHits': False,
                    'UseOnlyAxialCDCHits': False}

        elif finder_module_name in ("add_tracking_reconstruction", 'StandardReco', 'StandardTrackingReconstruction'):
            return {'UsePXDHits': 'PXD' in self.components,
                    'UseSVDHits': 'SVD' in self.components,
                    'UseCDCHits': 'CDC' in self.components,
                    'UseOnlyAxialCDCHits': False}

        elif finder_module_name == 'TrackFinderMCTruth':
            if isinstance(finder_module_or_name, basf2.Module):
                return {'UsePXDHits': self.get_module_param(finder_module_or_name, 'UsePXDHits'),
                        'UseSVDHits': self.get_module_param(finder_module_or_name, 'UseSVDHits'),
                        'UseCDCHits': self.get_module_param(finder_module_or_name, 'UseCDCHits'),
                        'UseOnlyAxialCDCHits': False}
            else:
                return {'UsePXDHits': 'PXD' in self.components,
                        'UseSVDHits': 'SVD' in self.components,
                        'UseCDCHits': 'CDC' in self.components,
                        'UseOnlyAxialCDCHits': False}
        else:
            get_logger().info('Could not determine tracking coverage for module name %s. '
                              'Using the default of the MCMatcherTracks',
                              finder_module_name)
            return {}

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        main_path = super(ReadOrGenerateTrackedEventsRun, self).create_path()

        # early return if only a simulation run was requested
        if self.simulate_only:
            return main_path

        # setting up fitting is only necessary when testing
        # track finding comonenst ex-situ
        if (self.fit_geometry and
                self.finder_module != 'StandardReco' and
                not isinstance(self.finder_module, StandardTrackingReconstructionModule)):

            # Prepare Genfit extrapolation
            setup_genfit_extrapolation_module = basf2.register_module('SetupGenfitExtrapolation')

            # only set if the default is not requested
            if not self.fit_geometry == "default":
                setup_genfit_extrapolation_module.param({'whichGeometry': self.fit_geometry})
            main_path.add_module(setup_genfit_extrapolation_module)

        if self.finder_module is not None:
            # Setup track finder
            # determine which sub-detector hits will be used
            tracking_coverage = self.determine_tracking_coverage(self.finder_module)

            if tracking_coverage.get("UseCDCHits") and "WireHitTopologyPreparer" not in main_path:
                main_path.add_module("WireHitTopologyPreparer")

            if self.finder_module == 'StandardReco':
                basf2.B2FATAL("Because of the transition to RecoTracks, this is not implemented in the moment!")
                track_finder_module = StandardTrackingReconstructionModule(components=self.components)
                main_path.add_module(track_finder_module)

            elif callable(self.finder_module):
                # finder module is a convenience function
                self.finder_module(main_path)

            else:
                track_finder_module = self.get_basf2_module(self.finder_module)
                main_path.add_module(track_finder_module)

            # TODO: The following modules do still need a list of genfit::TrackCands. We make the transition here
            main_path.add_module("GenfitTrackCandidatesCreator",
                                 genfitTrackCandsStoreArrayName=self.trackCandidatesColumnName)

            # check for detector geometry, necessary for track extrapolation in genfit
            if 'MCTrackMatcher' not in main_path and 'MCMatcherTracks' not in main_path:
                # Reference Monte Carlo tracks
                track_finder_mc_truth_module = basf2.register_module('TrackFinderMCTruth')
                track_finder_mc_truth_module.param({
                    'WhichParticles': ['primary'],
                    'EnergyCut': 0.1,
                    'GFTrackCandidatesColName': 'MCTrackCands'
                })
                track_finder_mc_truth_module.param(tracking_coverage)
                main_path.add_module(IfMCParticlesPresentModule(track_finder_mc_truth_module))

                # Track matcher
                mc_track_matcher_module = basf2.register_module('MCTrackMatcher')
                mc_track_matcher_module.param({
                    'MCGFTrackCandsColName': 'MCTrackCands',
                    'MinimalPurity': 0.66,
                    'RelateClonesToMCParticles': True,
                    'PRGFTrackCandsColName': self.trackCandidatesColumnName,
                })
                mc_track_matcher_module.param(tracking_coverage)
                main_path.add_module(IfMCParticlesPresentModule(mc_track_matcher_module))

        if self.fit_tracks:
            # Fit tracks
            gen_fitter_module = basf2.register_module('GenFitter')
            gen_fitter_module.param({'PDGCodes': [13]})
            main_path.add_module(gen_fitter_module)

        return main_path


class StandardReconstructionEventsRun(ReadOrGenerateTrackedEventsRun):
    generator_module = 'EvtGenInput'
    finder_module = 'StandardReco'


def main():
    generateTrackedEventsRun = StandardReconstructionEventsRun()

    # Allow presimulated events to be tracked in a second step
    allow_input = True
    argument_parser = generateTrackedEventsRun.create_argument_parser(allow_input=allow_input)

    argument_parser.add_argument('root_output_file',
                                 help='Output file to which the simulated events shall be written.')

    arguments = argument_parser.parse_args()

    # Configure the read or event generation from the command line
    generateTrackedEventsRun.configure(arguments)

    # Add the output module
    root_output_file_path = arguments.root_output_file
    root_output_module = basf2.register_module('RootOutput')
    root_output_params = {'outputFileName': root_output_file_path}
    root_output_module.param(root_output_params)

    generateTrackedEventsRun.add_module(root_output_module)

    # Execute the run
    generateTrackedEventsRun.execute()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    main()
