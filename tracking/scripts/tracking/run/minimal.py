#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os

import basf2

from . import utilities

import argparse
import logging
from ROOT import Belle2


def get_logger():
    return logging.getLogger(__name__)

# Provides a commandline interface to specify some parameters to an associated basf2 path execution


class EmptyRun(object):
    # Declarative section which can be redefined in a subclass
    description = "Empty execution of basf2"

    def __init__(self, **kwds):
        if kwds:
            raise ValueError("Received_unknown unknown argument")

    @property
    def name(self):
        return self.__class__.__name__

    def configure_and_execute_from_commandline(self):
        self.configure_from_commandline()
        self.execute()

    def execute(self):
        # Create path and run #
        #######################
        path = self.create_path()
        self.adjust_path(path)
        self.run(path)

    def run(self, path):
        # Run basf2 module path #
        #########################
        get_logger().info('Start processing')
        basf2.print_path(path)
        basf2.process(path)
        get_logger().info("\n%s", str(basf2.statistics))

    def configure_from_commandline(self):
        argument_parser = self.create_argument_parser()
        arguments = argument_parser.parse_args()
        self.configure(arguments)

    def configure(self, arguments):
        # Simply translate the arguments that have
        # the same name as valid instance arguments
        for (key, value) in list(vars(arguments).items()):
            if value is None:
                continue
            if hasattr(self, key):
                get_logger().info("Setting %s to %s", key, value)
                setattr(self, key, value)

    def create_argument_parser(self, **kwds):
        argument_parser = utilities.ArgumentParser(description=self.description, **kwds)
        return argument_parser

    def create_path(self):
        path = basf2.create_path()
        return path

    def adjust_path(self, path):
        """Hook that gives the opportunity to check the path for consistance before processing it"""
        pass

# Minimal run stub defining some general parameters


class MinimalRun(EmptyRun):
    description = "Minimally populated execution of basf2"

    # Declarative section which can be redefined in a subclass
    allow_input = True
    n_events = 10000
    root_input_file = None
    random_seed = None
    n_processes = 0
    n_events_to_skip = 0

    def create_argument_parser(self, **kwds):
        argument_parser = super().create_argument_parser(**kwds)
        master_argument_group = argument_parser.add_argument_group("Master arguments")

        if self.allow_input:
            master_argument_group.add_argument(
                '-i',
                '--input',
                default=argparse.SUPPRESS,
                dest='root_input_file',
                help='File path to the ROOT file from which the simulated events shall be loaded.'
            )

        master_argument_group.add_argument(
            '-n',
            '--events',
            dest='n_events',
            default=self.n_events,
            type=int,
            help='Number of events to be generated',
        )

        master_argument_group.add_argument(
            '-r',
            '--random-seed',
            dest='random_seed',
            default=argparse.SUPPRESS,
            type=str,
            help='The random number generator seed to be set before the processing starts.',
        )

        master_argument_group.add_argument(
            '-p',
            '--processes',
            dest='n_processes',
            default=argparse.SUPPRESS,
            type=int,
            help='The number of parallel processes to be used for processing.',
        )

        master_argument_group.add_argument(
            '--n-events-to-skip',
            dest='n_events_to_skip',
            default=argparse.SUPPRESS,
            type=int,
            help='The number of events to skip',
        )

        return argument_parser

    def create_path(self):
        path = super().create_path()

        if self.random_seed is not None:
            basf2.set_random_seed(self.random_seed)

        environment = Belle2.Environment.Instance()
        environment.setNumberEventsOverride(self.n_events)

        # If there is no input file is the EventInfoSetter as master module
        if not self.root_input_file:
            # Master module: EventInfoSetter
            path.add_module('EventInfoSetter',
                            evtNumList=[self.n_events],
                            runList=[1],
                            expList=[0],
                            skipNEvents=self.n_events_to_skip
                            )

        else:
            # Master module: RootInput
            path.add_module('RootInput',
                            inputFileName=self.root_input_file,
                            skipNEvents=self.n_events_to_skip)

        # Progress module
        path.add_module('Progress')

        if self.n_processes:
            environment = Belle2.Environment.Instance()
            environment.setNumberProcessesOverride(self.n_processes)

        return path
