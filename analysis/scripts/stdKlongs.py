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


def stdKlongs(listtype='veryLoose', path=analysis_main):
    """
    Function to prepare one of several standardized types of Klong lists, with
    the followung choices for ``listtype``:

    - 'all' all neutral KLMClusters with no cuts
    - 'veryLoose' (default) with some (very)loose quality selections
    - 'loose' with KlongID requirements
    - 'tight' like loose but with higher ID cut

    Parameters:
        listtype (str) name of standard list options as above
        param path (basf2.Path) modules are added to this path
    """

    # all KLM clusters
    if listtype == 'all':
        fillParticleList('K_L0:all', 'isFromKLM > 0', True, path)

    # loose KLs, removes buggy KLM clusters
    elif listtype == 'veryLoose':
        stdKlongs('all', path)
        cutAndCopyList(
            'K_L0:veryLoose',
            'K_L0:all',
            'E > 0.5 and E < 10. and klmClusterTiming > -10 and klmClusterTiming < 100.',
            True,
            path)

    # additional cuts on KL_ID
    elif listtype == 'loose':
        stdKlongs('all', path)
        cutAndCopyList(
            'K_L0:loose',
            'K_L0:all',
            'E > 0.5 and E < 10. and klmClusterTiming > -10 and klmClusterTiming < 100. and klongID_KLM > 0.04',
            True,
            path)

    # additional cuts on KL_ID
    elif listtype == 'tight':
        stdKlongs('loose', path)
        cutAndCopyList(
            'K_L0:tight',
            'K_L0:loose',
            'klongID_KLM > 0.2',
            True,
            path)


# Used in skimming code
def loadStdSkimKL0(path=analysis_main):
    """Load KLongs for skimming.

    Parameters:
        path (belle2.Path) modules are added to this path
    """
    stdKlongs('loose', path)
    cutAndCopyList(
        'K_L0:skim',
        'K_L0:loose',
        '',
        True,
        path)
