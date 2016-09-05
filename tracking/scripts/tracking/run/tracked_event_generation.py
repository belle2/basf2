#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
import tracking

from tracking.metamodules import IfMCParticlesPresentModule, IfStoreArrayPresentModule
from . import utilities
from .event_generation import ReadOrGenerateEventsRun

import logging


def get_logger():
    return logging.getLogger(__name__)


class ReadOrGenerateTrackedEventsRun(ReadOrGenerateEventsRun):
    #: Descriptio fof the run setup to be displayed on command line
    description = "Apply tracking to presimulated events or events generated on the fly."

    #: Name of the finder module to be used - can be everything that is accepted by tracking.run.utilities.extend_path
    finder_module = None

    #: States which detectors the finder module covers like as a dictionary like
    tracking_coverage = {
        'UsePXDHits': True,
        'UseSVDHits': True,
        'UseCDCHits': True,
        'UseOnlyAxialCDCHits': True
    }

    #: Add the track fitting to the execution
    fit_tracks = False

    #: Determines which fit geometry should be used.
    fit_geometry = None

    #: Name of the store array of RecoTracks
    trackCandidatesColumnName = 'RecoTracks'

    def create_argument_parser(self, **kwds):
        argument_parser = super().create_argument_parser(**kwds)

        tracking_argument_group = argument_parser.add_argument_group("Tracking setup arguments")

        tracking_argument_group.add_argument(
            '-f',
            '--finder',
            choices=utilities.NonstrictChoices(finder_modules_by_short_name.keys()),
            default=self.finder_module,
            dest='finder_module',
            help='Name of the finder module to be evaluated.',)

        tracking_argument_group.add_argument(
            '--fit',
            action="store_true",
            dest='fit_tracks',
            help='Apply the fitting to the found tracks'
        )

        # Indication with which geometry tracks should be fitted
        tracking_argument_group.add_argument(
            '--fit-geometry',
            choices=['TGeo', 'Geant4', 'default'],
            default=self.fit_geometry,
            dest='fit_geometry',
            help='Geometry to be used with Genfit. (Was used for earlier investigations geometries seem stable now)'
        )

        tracking_argument_group.add_argument(
            '-so',
            '--simulate-only',
            action='store_true',
            default=self.simulate_only,
            dest='simulate_only',
            help='Only generate and simulate the events, but do not run any tracking or validation code')

        return argument_parser

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        path = super().create_path()

        # early return if only a simulation run was requested
        if self.simulate_only:
            return path

        # setting up fitting is only necessary when testing
        # track finding comonenst ex-situ
        if self.fit_geometry or self.fit_tracks:
            if 'SetupGenfitExtrapolation' not in path:
                # Prepare Genfit extrapolation
                setup_genfit_extrapolation_module = path.add_module('SetupGenfitExtrapolation')
                # Only update the used geometry if the default is not requested
                if self.fit_geometry and self.fit_geometry != "default":
                    setup_genfit_extrapolation_module.param({'whichGeometry': self.fit_geometry})

        if self.finder_module is not None:
            # Setup track finder
            utilities.extend_path(path,
                                  self.finder_module,
                                  finder_modules_by_short_name,
                                  allow_function_import=True)

            # determine which sub-detector hits will be used
            tracking_coverage = self.tracking_coverage

            # Include the mc tracks if the monte carlo data is presentx
            if 'MCRecoTracksMatcher' not in path:
                # Reference Monte Carlo tracks
                track_finder_mc_truth_module = basf2.register_module('TrackFinderMCTruthRecoTracks')
                track_finder_mc_truth_module.param({
                    'WhichParticles': ['primary'],
                    'EnergyCut': 0.1,
                    'RecoTracksStoreArrayName': 'MCRecoTracks',
                    **tracking_coverage
                })

                # Track matcher
                mc_track_matcher_module = basf2.register_module('MCRecoTracksMatcher')
                mc_track_matcher_module.param({
                    'mcRecoTracksStoreArrayName': 'MCRecoTracks',
                    'MinimalPurity': 0.66,
                    'RelateClonesToMCParticles': True,
                    'prRecoTracksStoreArrayName': "RecoTracks",
                    **tracking_coverage
                })

                path.add_module(IfMCParticlesPresentModule(track_finder_mc_truth_module))
                path.add_module(IfMCParticlesPresentModule(mc_track_matcher_module))

        if self.fit_tracks:
            # Fit tracks
            gen_fitter_module = basf2.register_module('DAFRecoFitter')
            gen_fitter_module.param({'pdgCodesToUseForFitting': [13]})
            path.add_module(gen_fitter_module)
            trackbuilder = basf2.register_module('TrackCreator', defaultPDGCode=13)
            path.add_module(trackbuilder)

        return path


def add_standard_finder(path):
    components = []
    for module in path.modules():
        if module.type() == "Geometry":
            components = utilities.get_module_param(module, "components")
    tracking.add_track_finding(path, components=components)


finder_modules_by_short_name = {
    'MC': 'TrackFinderMCTruthRecoTracks',
    'TrackFinder': add_standard_finder,
    'TrackFinderCDC': tracking.add_cdc_track_finding,
    'TrackFinderVXD': tracking.add_vxd_track_finding,
    'TrackFinderCDCLegendre': lambda path: (path.add_module('WireHitTopologyPreparer'),
                                            path.add_module('TrackFinderCDCLegendreTracking')),
    'SegmentFinderCDC': lambda path: (path.add_module('WireHitTopologyPreparer'),
                                      path.add_module('SegmentFinderCDCFacetAutomaton')),
}


class StandardReconstructionEventsRun(ReadOrGenerateTrackedEventsRun):
    generator_module = 'EvtGenInput'

    def finder_module(self, path):
        tracking.add_tracking_reconstruction(path, components=self.components)
