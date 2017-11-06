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
        'UseOnlyAxialCDCHits': False,
        'UseOnlyBeforeTOP': True,
        'UseReassignedHits': True,
        'UseNLoops': 1,
        'WhichParticles': [],
    }

    #: Add the track fitting to the execution
    fit_tracks = False

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
            default=self.fit_tracks,
            dest='fit_tracks',
            help='Apply the fitting to the found tracks'
        )

        return argument_parser

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        path = super().create_path()

        # setting up fitting is only necessary when testing
        # track finding comonenst ex-situ
        if self.fit_tracks:
            if 'SetupGenfitExtrapolation' not in path:
                # Prepare Genfit extrapolation
                path.add_module('SetupGenfitExtrapolation')

        if self.finder_module is not None:
            # Setup track finder
            utilities.extend_path(path,
                                  self.finder_module,
                                  finder_modules_by_short_name,
                                  allow_function_import=True)

            # determine which sub-detector hits will be used
            tracking_coverage = dict(self.tracking_coverage)

            # Include the mc tracks if the monte carlo data is presentx
            if 'MCRecoTracksMatcher' not in path:
                matching_coverage = {key: value for key, value in tracking_coverage.items()
                                     if key in ('UsePXDHits', 'UseSVDHits', 'UseCDCHits', 'MinimalEfficiency', 'MinimalPurity')}

                # Removing minimal efficiency and purity as they are only parameters of the matching
                if "MinimalEfficiency" in tracking_coverage:
                    tracking_coverage.pop("MinimalEfficiency")
                if "MinimalPurity" in tracking_coverage:
                    tracking_coverage.pop("MinimalPurity")

                # Reference Monte Carlo tracks
                track_finder_mc_truth_module = basf2.register_module('TrackFinderMCTruthRecoTracks')
                track_finder_mc_truth_module.param({
                    'RecoTracksStoreArrayName': 'MCRecoTracks',
                    **tracking_coverage
                })

                # Track matcher
                mc_track_matcher_module = basf2.register_module('MCRecoTracksMatcher')

                mc_track_matcher_module.param({
                    'mcRecoTracksStoreArrayName': 'MCRecoTracks',
                    'MinimalPurity': 0.66,
                    'prRecoTracksStoreArrayName': "RecoTracks",
                    **matching_coverage
                })

                path.add_module(IfMCParticlesPresentModule(track_finder_mc_truth_module))
                path.add_module(IfMCParticlesPresentModule(mc_track_matcher_module))

        if self.fit_tracks:
            # Fit tracks
            gen_fitter_module = basf2.register_module('DAFRecoFitter')
            gen_fitter_module.param({'pdgCodesToUseForFitting': [13]})
            path.add_module(gen_fitter_module)
            trackbuilder = basf2.register_module('TrackCreator', pdgCodes=[13])
            path.add_module(trackbuilder)

        return path


def add_standard_finder(path):
    import tracking
    components = None
    for module in path.modules():
        if module.type() == "Geometry":
            components = utilities.get_module_param(module, "components")
    if not components:
        components = None

    if 'SetupGenfitExtrapolation' not in path:
        path.add_module('SetupGenfitExtrapolation', energyLossBrems=False, noiseBrems=False)

    tracking.add_track_finding(path, components=components)


def add_cosmics_finder(path):
    import tracking
    components = None
    for module in path.modules():
        if module.type() == "Geometry":
            components = utilities.get_module_param(module, "components")
    if not components:
        components = None

    if 'SetupGenfitExtrapolation' not in path:
        path.add_module('SetupGenfitExtrapolation', energyLossBrems=False, noiseBrems=False)

    tracking.add_cr_tracking_reconstruction(path, components=components)


def add_standard_reconstruction(path):
    import reconstruction
    components = None
    for module in path.modules():
        if module.type() == "Geometry":
            components = utilities.get_module_param(module, "components")
    if not components:
        components = None
    reconstruction.add_reconstruction(path, components=components)


def add_cosmics_reconstruction(path):
    import reconstruction
    components = None
    for module in path.modules():
        if module.type() == "Geometry":
            components = utilities.get_module_param(module, "components")
    if not components:
        components = None
    reconstruction.add_cosmics_reconstruction(path, components=components)

finder_modules_by_short_name = {
    'MC': 'TrackFinderMCTruthRecoTracks',
    'Reconstruction': add_standard_reconstruction,
    'CosmicsReconstruction': add_cosmics_reconstruction,
    'TrackFinder': add_standard_finder,
    'CosmicsTrackFinder': add_cosmics_finder,
    'TrackFinderVXD': tracking.add_vxd_track_finding,
    'TFCDC': lambda path: tracking.add_cdc_track_finding(path, with_ca=True),
    'TFCDC_Cosmics': lambda path: tracking.add_cdc_cr_track_finding(path),
    'TFCDC_Global': tracking.add_cdc_track_finding,
    'TFCDC_Ca': lambda path: (path.add_module('TFCDC_WireHitPreparer',
                                              flightTimeEstimation="outwards"),
                              path.add_module('TFCDC_ClusterPreparer',
                                              SuperClusterDegree=3,
                                              SuperClusterExpandOverApogeeGap=True),
                              path.add_module('TFCDC_SegmentFinderFacetAutomaton'),
                              path.add_module("TFCDC_TrackFinderSegmentPairAutomaton"),
                              path.add_module("TFCDC_TrackCreatorSingleSegments",
                                              MinimalHitsBySuperLayerId={0: 15}),
                              path.add_module('TFCDC_TrackExporter')),
    'TFCDC_Axial': lambda path: (path.add_module('TFCDC_WireHitPreparer',
                                                 flightTimeEstimation="outwards"),
                                 path.add_module('TFCDC_ClusterPreparer'),
                                 path.add_module('TFCDC_AxialTrackFinderLegendre'),
                                 path.add_module('TFCDC_TrackExporter')),
    'TFCDC_Segments': lambda path: (path.add_module('TFCDC_WireHitPreparer',
                                                    flightTimeEstimation="outwards"),
                                    path.add_module('TFCDC_ClusterPreparer'),
                                    path.add_module('TFCDC_SegmentFinderFacetAutomaton'),
                                    path.add_module('TFCDC_TrackCreatorSingleSegments',
                                                    MinimalHitsBySuperLayerId={sl_id: 0 for sl_id in range(9)}),
                                    path.add_module('TFCDC_TrackExporter')),
    'TFCDC_MCSegments': lambda path: (path.add_module('TFCDC_WireHitPreparer',
                                                      flightTimeEstimation="outwards"),
                                      path.add_module('TFCDC_SegmentCreatorMCTruth'),
                                      path.add_module('TFCDC_SegmentLinker',
                                                      segments="CDCSegment2DVector",
                                                      filter="truth"),
                                      path.add_module('TFCDC_TrackCreatorSingleSegments',
                                                      MinimalHitsBySuperLayerId={sl_id: 0 for sl_id in range(9)}),
                                      path.add_module('TFCDC_TrackExporter')),
    'FirstLoop': lambda path: path.add_module('TFCDC_WireHitPreparer', UseNLoops=1.0),
}


class StandardReconstructionEventsRun(ReadOrGenerateTrackedEventsRun):
    generator_module = 'EvtGenInput'

    def finder_module(self, path):
        tracking.add_tracking_reconstruction(path, components=self.components)
