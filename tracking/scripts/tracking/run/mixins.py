#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from tracking.modules import BrowseTFileOnTerminateModule
from tracking.run.minimal import MinimalRun


import logging


def get_logger():
    return logging.getLogger(__name__)


class RunMixin(MinimalRun):
    pass


class BrowseTFileOnTerminateRunMixin(RunMixin):
    output_file_name = None
    show_results = False

    def create_argument_parser(self, **kwds):
        argument_parser = super(BrowseTFileOnTerminateRunMixin, self).create_argument_parser(**kwds)

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
        main_path = super(BrowseTFileOnTerminateRunMixin, self).create_path()

        if self.show_results and self.output_file_name:
            browse_tfile_on_terminate_module = BrowseTFileOnTerminateModule(self.output_file_name)
            main_path.add_module(browse_tfile_on_terminate_module)

        return main_path
