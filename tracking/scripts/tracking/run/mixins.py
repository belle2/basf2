#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from .minimal import EmptyRun
from tracking.modules import BrowseTFileOnTerminateModule

import logging


def get_logger():
    return logging.getLogger(__name__)


class RunMixin(EmptyRun):
    pass


class BrowseTFileOnTerminateRunMixin(RunMixin):
    output_file_name = None
    show_results = False

    def create_argument_parser(self, **kwds):
        argument_parser = super().create_argument_parser(**kwds)

        argument_parser.add_argument(
            '-s',
            '--show',
            action='store_true',
            default=self.show_results,
            dest='show_results',
            help='Show generated plots in a TBrowser immediatly.',)

        return argument_parser

    def create_path(self):
        # Sets up a path that plays back pregenerated events or generates events
        # based on the properties in the base class.
        path = super().create_path()

        if self.show_results and self.output_file_name:
            browse_tfile_on_terminate_module = BrowseTFileOnTerminateModule(self.output_file_name)
            path.add_module(browse_tfile_on_terminate_module)

        return path


class RootOutputRunMixin(RunMixin):
    root_output_file = None

    def create_argument_parser(self, **kwds):
        argument_parser = super().create_argument_parser(**kwds)
        argument_parser.add_argument(
            'root_output_file',
            help='Output file to which the simulated events shall be written.'
        )

        return argument_parser

    def create_path(self):
        path = super().create_path()

        path.add_module(
            'RootOutput',
            outputFileName=self.root_output_file
        )

        return path
