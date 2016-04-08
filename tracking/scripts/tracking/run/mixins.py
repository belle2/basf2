#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from tracking.modules import BrowseTFileOnTerminateModule
from tracking.run.minimal import MinimalRun


class RunMixin(MinimalRun):
    """Base class for mixins during run. Does nothing for the moment."""
    pass


class BrowseTFileOnTerminateRunMixin(RunMixin):
    """Class to mix in a BrowseTFileOnTerminateModule and a parameter show_results,
       which shows the output fule name after the calculations or not."""

    #: Name of the output file name to show
    output_file_name = None
    #: Flag to show results or not
    show_results = False

    def create_argument_parser(self, **kwds):
        """Add the show_results argument as -s and --show flag."""
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
        """Add the BrowseTFileOnTerminateModule module to the path if show_results is True and the
           output_file_name is set."""
        main_path = super(BrowseTFileOnTerminateRunMixin, self).create_path()

        if self.show_results and self.output_file_name:
            browse_tfile_on_terminate_module = BrowseTFileOnTerminateModule(self.output_file_name)
            main_path.add_module(browse_tfile_on_terminate_module)

        return main_path
