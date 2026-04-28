#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import B2FATAL, B2WARNING
from modularAnalysis import fillParticleList


def stdKlongs(listtype='allklm', path=None):
    """
    Warning:
        This function is a placeholder for Klong selections. Currently
        everything but the 'allklm' and 'allecl' lists is disabled pending study.

    By default, prepares the 'K_L0:allklm' list with no cuts (all KLM clusters are loaded).
    It's possible to provide the argument 'allecl' to create a list of all ECL clusters loaded as Klong candidates.

    Parameters:
        listtype (str): name of standard list options (currently only
            'allklm' and 'allecl' are supported/recommended)
        path (basf2.Path): modules are added to this path
    """

    # all KLM clusters
    if listtype == 'allklm':
        B2WARNING('The Klong particles in the list "allklm" are exclusively built from KLMClusters!')
        fillParticleList('K_L0:allklm', '[isFromKLM > 0]', True, path)
    # all ECL clusters
    elif listtype == 'allecl':
        B2WARNING('The Klong particles in the list "allecl" are exclusively built from ECLClusters!')
        fillParticleList('K_L0:allecl', '[isFromECL > 0]', True, path)
    else:
        B2FATAL("""

    Only the particle lists 'allklm' (Klongs built from KLM clusters) and 'allecl' (Klongs built from neutral ECLCluster) are
    currently supported. Please use:

    stdKlongs('allklm', path=mypath)
            """)
