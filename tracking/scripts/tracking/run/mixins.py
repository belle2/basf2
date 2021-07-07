#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from tracking.run.minimal import EmptyRun
import tracking.root_utils as root_utils

import logging


def get_logger():
    return logging.getLogger(__name__)


class RunMixin(EmptyRun):
    pass


class PostProcessingRunMixin(EmptyRun):
    """Post-process the basf2 job output"""

    #: By default, browse the output TFile too
    postprocess_only = False

    def create_argument_parser(self, **kwds):
        """Parse the command-line post-processing arguments"""
        argument_parser = super().create_argument_parser(**kwds)
        postprocess_argument_group = argument_parser.add_argument_group("Postprocessing arguments")
        postprocess_argument_group.add_argument(
            '-po',
            '--postprocess-only',
            action='store_true',
            default=self.postprocess_only,
            dest='postprocess_only',
            help='Only run the post processing of this run.',)

        return argument_parser

    def run(self, path):
        """Post-process the basf2 job output"""
        if not self.postprocess_only:
            super().run(path)

        self.postprocess()

    def postprocess(self):
        """By default, do nothing. (may be overridden)"""


class BrowseTFileOnTerminateRunMixin(PostProcessingRunMixin):
    """Browse interactively the basf2 job output"""

    #: There is no default for the name of the output TFile
    output_file_name = None
    #: By default, do not show the browsing results
    show_results = False

    def create_argument_parser(self, **kwds):
        """Parse the command-line TFile-browsing arguments"""
        argument_parser = super().create_argument_parser(**kwds)

        postprocess_argument_group = argument_parser
        for group in argument_parser._action_groups:
            if group.title == "Postprocessing arguments":
                postprocess_argument_group = group
                break

        postprocess_argument_group.add_argument(
            '-s',
            '--show',
            action='store_true',
            default=self.show_results,
            dest='show_results',
            help='Show generated plots in a TBrowser immediatly.',)

        return argument_parser

    def postprocess(self):
        """Browse the TFile interactively"""
        if self.show_results and self.output_file_name:
            with root_utils.root_open(self.output_file_name) as tfile:
                root_utils.root_browse(tfile)
                input("Close with return key.")

        super().postprocess()


class RootOutputRunMixin(RunMixin):
    """Configure for basf2 job output ROOT TFile"""

    #: There is no default for the name of the output TFile
    root_output_file = None

    def create_argument_parser(self, **kwds):
        """Parse the command-line output-file-specification argument"""
        argument_parser = super().create_argument_parser(**kwds)
        argument_parser.add_argument(
            'root_output_file',
            help='Output file to which the simulated events shall be written.'
        )

        return argument_parser

    def create_path(self):
        """Create a new basf2 path and add the RootOutput module to it"""
        path = super().create_path()

        path.add_module(
            'RootOutput',
            outputFileName=self.root_output_file
        )

        return path
