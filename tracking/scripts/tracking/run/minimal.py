#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import basf2

from tracking.run import utilities

import argparse
import logging
from ROOT import Belle2


def get_logger():
    return logging.getLogger(__name__)

# Provides a commandline interface to specify some parameters to an associated basf2 path execution


class EmptyRun(object):
    """Configure for a bare-bones basf2 job"""

    # Declarative section which can be redefined in a subclass

    #: Description of the run setup to be displayed on command line
    description = "Empty execution of basf2"

    def __init__(self, **kwds):
        """Constructor"""
        if kwds:
            raise ValueError("Received_unknown unknown argument")

    @property
    def name(self):
        """provide name of this object"""
        return self.__class__.__name__

    def configure_and_execute_from_commandline(self):
        """Configure basf2 job script from command-line arguments then run it"""
        self.configure_from_commandline()
        self.execute()

    def execute(self):
        """Create the basf2 path then run the job"""
        # Create path and run #
        #######################
        path = self.create_path()
        self.adjust_path(path)
        self.run(path)

    def run(self, path):
        """Process the basf2 path"""
        # Run basf2 module path #
        #########################
        get_logger().info('Start processing')
        basf2.print_path(path)
        basf2.process(path)
        get_logger().info("\n%s", str(basf2.statistics))

    def configure_from_commandline(self):
        """Convert the command-line arguments to a basf2 job script"""
        argument_parser = self.create_argument_parser()
        arguments = argument_parser.parse_args()
        self.configure(arguments)

    def configure(self, arguments):
        """Save the command-line arguments as key-value pairs"""
        # Simply translate the arguments that have
        # the same name as valid instance arguments
        for (key, value) in list(vars(arguments).items()):
            if value is None:
                continue
            if hasattr(self, key):
                get_logger().info("Setting %s to %s", key, value)
                setattr(self, key, value)

    def create_argument_parser(self, **kwds):
        """Parse the command-line arguments to a basf2 job script"""
        argument_parser = utilities.ArgumentParser(description=self.description, **kwds)
        return argument_parser

    def create_path(self):
        """Create a new basf2 path"""
        path = basf2.create_path()
        return path

    def adjust_path(self, path):
        """Hook that gives the opportunity to check the path for consistency before processing it"""

# Minimal run stub defining some general parameters


class MinimalRun(EmptyRun):
    """Configure for a minimal basf2 job"""

    #: Description of this object
    description = "Minimally populated execution of basf2"

    # Declarative section which can be redefined in a subclass

    #: By default, this basf2 job can read events from an input ROOT TFile
    allow_input = True
    #: By default, process 10000 events
    n_events = 10000
    #: By default, there is no input ROOT TFile
    root_input_file = None
    #: By default, the random-number seed is unassigned
    random_seed = None
    #: By default, no parallel processing
    n_processes = 0
    #: By default, do not skip any events at the start of the input ROOT TFile
    n_events_to_skip = 0

    def create_argument_parser(self, **kwds):
        """Convert command-line arguments to basf2 argument list"""
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
        """Create and configure the basf2 path"""
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
