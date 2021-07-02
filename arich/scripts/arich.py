# !/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
This module defines wrapper functions around the arich modules.
"""

from basf2 import register_module
import variables.utils as vu


def arichVariablesToNtuple(decayString, variables, arichSelector, treename='variables', filename='ntuple.root', path=None):
    """
    Creates and fills a flat ntuple with the specified variables from the VariableManager.
    If a decayString is provided, then there will be one entry per candidate (for particle in list of candidates).
    If an empty decayString is provided, there will be one entry per event (useful for trigger studies, etc).

    Parameters:
        decayString (str): specifies type of Particles and determines the name of the ParticleList
        variables (list(str)): the list of variables (which must be registered in the VariableManager)
        arichSelector (str): decay string that marks the particles to which arich info should be appended
        treename (str): name of the ntuple tree
        filename (str): which is used to store the variables
        path (basf2.Path): the basf2 path where the analysis is processed
    """

    arich_vars = vu.create_aliases_for_selected(['arich'],
                                                decay_string=arichSelector)

    output = register_module('arichToNtuple')
    output.set_name('arichToNtuple_' + decayString)
    output.param('particleList', decayString)
    output.param('variables', variables)
    output.param('arichVariables', arich_vars)
    output.param('arichSelector', arichSelector)
    output.param('fileName', filename)
    output.param('treeName', treename)
    path.add_module(output)
