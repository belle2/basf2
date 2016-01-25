#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os

import basf2
import simulation

import tracking.utilities as utilities
import tracking.metamodules as metamodules

import logging
from ROOT import Belle2


def get_logger():
    return logging.getLogger(__name__)


# Minimal run stub #
####################

# This also provides a commandline interface to specify some parameters.

class MinimalRun(object):
    # Declarative section which can be redefined in a subclass
    n_events = 10000
    root_input_file = None
    components = []
    random_seed = None
    n_processes = 0

    def __init__(self):
        super(MinimalRun, self).__init__()
        self._path = None

    @property
    def name(self):
        return self.__class__.__name__

    def create_argument_parser(self, allow_input=True, **kwds):
        # Argument parser that gives a help full message on error,
        # which includes the options that are valid.
        argument_parser = utilities.DefaultHelpArgumentParser(**kwds)

        if allow_input:
            argument_parser.add_argument(
                '-i',
                '--input',
                default=self.root_input_file,
                dest='root_input_file',
                help='File path to the ROOT file from which the simulated events shall be loaded.'
            )

        argument_parser.add_argument(
            '-c',
            '--component',
            dest='components',
            default=None,
            action='append',
            help=('Add component. Multiple repeatition adds more components.'
                  'If not given use the default settings of the run: %s' % type(self).components)
        )

        argument_parser.add_argument(
            '-n',
            '--events',
            dest='n_events',
            default=self.n_events,
            type=int,
            help='Number of events to be generated',
        )

        argument_parser.add_argument(
            '-r',
            '--random-seed',
            dest='random_seed',
            default=self.random_seed,
            type=int,
            help='The random number generator seed to be set before the processing starts.',
        )

        argument_parser.add_argument(
            '-p',
            '--processes',
            dest='n_processes',
            default=self.n_processes,
            type=int,
            help='The number of parallel processes to be used for processing.',
        )

        return argument_parser

    def forward_random_seed(self):
        if self.random_seed is not None:
            get_logger().info("Setting random seed to %s" % self.random_seed)
            basf2.set_random_seed(self.random_seed)

    def configure(self, arguments):
        # Simply translate the arguments that have
        # the same name as valid instance arguments
        for (key, value) in list(vars(arguments).items()):
            if value is None:
                continue
            if hasattr(self, key):
                setattr(self, key, value)
                get_logger().info("Setting %s to %s", key, value)

        self.forward_random_seed()

    def configure_from_commandline(self):
        argument_parser = self.create_argument_parser()
        arguments = argument_parser.parse_args()
        self.configure(arguments)

    def create_path(self):
        # Compose basf2 module path #
        #############################
        main_path = basf2.create_path()

        # If there is no input file is the EventInfoSetter as master module
        if self.root_input_file is None:
            # Master module: EventInfoSetter
            event_info_setter_module = basf2.register_module('EventInfoSetter')
            event_info_setter_module.param({'evtNumList': [self.n_events],
                                            'runList': [1], 'expList': [1]})
            main_path.add_module(event_info_setter_module)

        else:
            # Master module: RootInput
            root_input_module = basf2.register_module('RootInput')
            root_input_module.param({'inputFileName': self.root_input_file})
            main_path.add_module(root_input_module)

            environment = Belle2.Environment.Instance()
            environment.setNumberEventsOverride(self.n_events)

        # Progress module
        progress_module = basf2.register_module('Progress')
        main_path.add_module(progress_module)

        if self.n_processes:
            environment = Belle2.Environment.Instance()
            environment.setNumberProcessesOverride(self.n_processes)

        # gearbox & geometry needs to be registered any way
        gearbox_module = basf2.register_module('Gearbox')
        main_path.add_module(gearbox_module)

        components = self.components
        geometry_module = basf2.register_module('Geometry')
        geometry_module.param('components', components)
        main_path.add_module(geometry_module)

        return main_path

    @property
    def path(self):
        if self._path is None:
            self._path = self.create_path()
        return self._path

    def add_module(self, module=None):
        path = self.path
        module = self.get_basf2_module(module)
        path.add_module(module)

    def execute(self):
        # Create path and run #
        #######################
        main_path = self.path

        # Run basf2 module path #
        #########################
        get_logger().info('Start processing')
        basf2.process(main_path)
        get_logger().info("\n%s", str(basf2.statistics))

    def configure_and_execute_from_commandline(self):
        self.configure_from_commandline()
        self.execute()

    @classmethod
    def get_basf2_module(cls, module_or_module_name, **kwargs):
        if isinstance(module_or_module_name, list):
            if kwargs:
                raise ValueError("kwargs keyword is not supported for module lists.")
            modules = module_or_module_name
            modules = [cls.get_basf2_module(m) for m in modules]
            return metamodules.PathModule(modules=modules)
        elif isinstance(module_or_module_name, str):
            module_name = module_or_module_name
            module = basf2.register_module(module_name)
            module.param(kwargs)
            return module
        elif isinstance(module_or_module_name, basf2.Module):
            module = module_or_module_name
            module.param(kwargs)
            return module
        else:
            message_template = \
                '%s of type %s is neither a module nor the name of module. Expected str or basf2.Module instance.'
            raise ValueError(message_template % (module_or_module_name,
                                                 type(module_or_module_name)))

    @classmethod
    def get_basf2_module_name(cls, module_or_module_name):
        if isinstance(module_or_module_name, list):
            return "PathModule"
        elif isinstance(module_or_module_name, str):
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
