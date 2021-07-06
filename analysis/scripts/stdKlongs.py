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
        everything but the 'allklm' list is disabled pending study.

    Prepares the 'K_L0:allklm' list with no cuts (all KLM clusters are loaded).

    Parameters:
        listtype (str): name of standard list options (currently only
            'all' is supported/recommended)
        path (basf2.Path): modules are added to this path
    """

    # all KLM clusters
    if listtype == 'allklm':
        B2WARNING('The Klong particles in the list "allklm" are exclusively built from KLMClusters!')
        fillParticleList('K_L0:allklm', '[isFromKLM > 0] and [klmClusterKlId >= 0] and [klmClusterKlId <= 1]', True, path)
    elif listtype == 'allecl':
        B2WARNING('The Klong particles in the list "allecl" are exclusively built from ECLClusters!')
        fillParticleList('K_L0:allecl', 'isFromECL > 0', True, path)
    else:
        B2FATAL("""

    Only the particle lists 'allklm' (Klongs built from KLM clusters) and 'allecl' (Klongs built from neutral ECLCluster) are
    currently supported. Please use:

    stdKlongs('allklm', path=mypath)
            """)
#    # loose KLs, removes buggy KLM clusters
#    elif listtype == 'veryLoose':
#        stdKlongs('all', path)
#        selection = 'E > 0.5 and E < 10. and klmClusterTiming > -10 and klmClusterTiming < 100.'
#        B2WARNING("The standard Klong lists are not studied or optimised yet. ")
#        B2WARNING("Beware that anything more complex than the 'all' list may not work as desired (or at all).")
#        B2WARNING("You will have the following cuts applied: %s" % selection)
#        cutAndCopyList(
#            'K_L0:veryLoose',
#            'K_L0:all',
#            selection,
#            True,
#            path)
#
#    # additional cuts on KL_ID
#    elif listtype == 'loose':
#        stdKlongs('all', path)
#        selection = 'E > 0.5 and E < 10. and klmClusterTiming > -10 and klmClusterTiming < 100. and klmClusterKlId > 0.04'
#        B2WARNING("The standard Klong lists are not studied or optimised yet. ")
#        B2WARNING("Beware that anything more complex than the 'all' list may not work as desired (or at all).")
#        B2WARNING("You will have the following cuts applied: %s" % selection)
#        cutAndCopyList(
#            'K_L0:loose',
#            'K_L0:all',
#            selection,
#            True,
#            path)
#
#    # additional cuts on KL_ID
#    elif listtype == 'tight':
#        stdKlongs('loose', path)
#        tight_selection = 'klmClusterKlId > 0.2'
#        B2WARNING("With the following additional tight selection: %s" % tight_selection)
#        cutAndCopyList(
#            'K_L0:tight',
#            'K_L0:loose',
#            tight_selection,
#            True,
#            path)
