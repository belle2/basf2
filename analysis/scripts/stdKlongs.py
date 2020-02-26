#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
#
# Author(s): B.Oberhof (benjamin.oberhof@lnf.infn.it)
#
# more documentation:
# https://confluence.desy.de/display/BI/Klong+ID
#
########################################################

from modularAnalysis import fillParticleList, cutAndCopyList


def stdKlongs(listtype='all', path=None):
    """
    Warning:
        This function is a placeholder for Klong selections. Currently
        everything but the 'all' list is disabled pending study.

    Prepares the 'K_L0:all' list with not cuts (all KLM clusters are loaded).

    Parameters:
        listtype (str): name of standard list options (currently only
            'all' is supported/recommended)
        path (basf2.Path): modules are added to this path
    """

    # all KLM clusters
    if listtype == 'all':
        fillParticleList('K_L0:all', 'isFromKLM > 0', True, path)
    else:
        B2WARNING("Only the 'all' list is currently recommended.")
        B2WARNING("Ignoring the requested type: %s and instead loading the 'all' list" % listtype)
        stdKlongs('all')

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


# Used in skimming code
def loadStdSkimKL0(path):
    """Load KLongs for skimming.

    Parameters:
        path (basf2.Path): modules are added to this path
    """
    stdKlongs('loose', path)
    cutAndCopyList(
        'K_L0:skim',
        'K_L0:loose',
        '',
        True,
        path)
