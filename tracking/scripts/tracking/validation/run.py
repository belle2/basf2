#!/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2
import simulation

from tracking.validation.module import TrackingValidationModule

TRACKING_MAILING_LIST = 'tracking@belle2.kek.jp'


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


