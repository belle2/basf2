#!/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2
# necessary for the logging to work properly
from basf2 import *
import simulation
from reconstruction import add_reconstruction

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
    root_input_file = None
    components = ['PXD', 'SVD', 'CDC', 'BeamPipe',
                  'MagneticFieldConstant4LimitedRCDC']
    finder_module = None  # To be specified
    tracking_coverage = {'UsePXDHits': True, 'UseSVDHits': True,
                         'UseCDCHits': True}
    run_simulation = True
    fit_geometry = None
    pulls = True
    contact = TRACKING_MAILING_LIST
    output_file_name = 'StandardTrackingValidation.root'

    # Allow override from instances only in these field names to prevent some spelling errors
    __slots__ = [
        'n_events',
        'generator_module',
        'root_input_file',
        'components',
        'finder_module',
        'run_simulation',
        'tracking_coverage',
        'fit_geometry',
        'contract',
        'output_file_name']

    @property
    def name(self):
        return self.__class__.__name__

    def determineTrackingCoverage(self, finder_module):
        if finder_module == "CDCLocalTracking" or finder_module == "CDCLegendreTracking":
            return {'UsePXDHits': False, 'UseSVDHits': False, 'UseCDCHits': True}
        elif finder_module == "VXDTF":
            return {'UsePXDHits': True, 'UseSVDHits': True, 'UseCDCHits': False}
        elif finder_module == "FullReco":
            return {'UsePXDHits': True, 'UseSVDHits': True, 'UseCDCHits': True}
        else:
            basf2.B2FATAL("Track finder module " + finder_module + " is not supported.")

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

        # use Generator if no root input file is specified
        if self.root_input_file is None:
            generatorModule = get_basf2_module(self.generator_module)
            main_path.add_module(generatorModule)
        else:
            rootInputModule = get_basf2_module('RootInput')
            rootInputModule.param({'inputFileName': self.root_input_file})
            main_path.add_module(rootInputModule)

        # Simulation
        print "self.run_simulation = " + str(self.run_simulation)
        components = self.components
        if self.run_simulation:
            simulation.add_simulation(main_path, components=components)
        else:
            # gearbox & geometry needs to be registered any way
            gearbox = get_basf2_module('Gearbox')
            main_path.add_module(gearbox)
            geometry = get_basf2_module('Geometry')
            geometry.param('components', components)
            main_path.add_module(geometry)

        # Setup track finder
        # determine which sub-detector hits will be used
        self.tracking_coverage = self.determineTrackingCoverage(self.finder_module)

        if self.finder_module == "FullReco":
            # add the full reconstruction chain
            add_reconstruction(main_path, components)
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
                                       'GFTrackCandidatesColName': 'MCTrackCands'})
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
