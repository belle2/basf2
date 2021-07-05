#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

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


class NonstrictChoices(list):

    """Class that instances can be given to an argparse.ArgumentParser.add_argument as choices keyword argument.

    The explicit choices stated during construction of this object are just suggestions but all other values are
    excepted as well.
    """

    def __contains__(self, value):
        """Test for correctness of the choices.
        Always returns true since all choices should be valid not only the ones stated at construction of this object.
        """
        return True

    def __iter__(self):
        """Displays all explicit values and a final "..." to indicate more choices might be possible."""
        # Append an ellipses to indicate that there are more choices.
        copy = list(super(NonstrictChoices, self).__iter__())
        copy.append('...')
        return iter(copy)

    def __str__(self):
        """Displays all explicit values and a final "..." to indicate more choices might be possible."""
        # Append an ellipses to indicate that there are more choices.
        copy = list(self)
        copy.append('...')
        return str(copy)
