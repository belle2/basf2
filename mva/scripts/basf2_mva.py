#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


def vector(*args):
    '''
    Create a std::vector to be used in PyROOT.
    '''
    import ROOT  # noqa
    vector = ROOT.std.vector(type(args[0]))()
    for v in args:
        vector.push_back(v)
    return vector
