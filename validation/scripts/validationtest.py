#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import ROOT


def add_properties(object, dictionary):
    """ Adds metaoptions to an object. """
    for key, value in dictionary.items():
        object.GetListOfFunctions().Add(ROOT.TNamed(key, value))
