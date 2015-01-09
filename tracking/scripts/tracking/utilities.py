#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys

import argparse


class DefaultHelpArgumentParser(argparse.ArgumentParser):

    """An argparse.Argument parse slightly changed such 
    that it always prints an extended help message incase of a parsing error."""

    def error(self, message):
        """Method invoked when a parsing error occured.
        Writes an extended help over the base ArgumentParser.
        """

        self.print_help()
        sys.stderr.write('error: %s\n' % message)
        sys.exit(2)


