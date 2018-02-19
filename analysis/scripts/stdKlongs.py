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

from basf2 import *
from modularAnalysis import *


def stdKlongs(listtype='veryLoose', path=analysis_main):
    """
    Function to prepare one of several standardized types of Klong lists:

    - 'K_L0:all' with no cuts
    - 'K_L0:veryLoose' (default) with some (very)loose quality selections
    - 'K_L0:loose' with KlongID requirements
    - 'K_L0:tight' like loose but with higher ID cut

    @param listtype name of standard list
    @param path     modules are added to this path
    """

    # all KLM clusters
    if listtype == 'all':
        fillParticleList('K_L0:all', '', True, path)

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
    stdKlongs('loose', path)
    cutAndCopyList(
        'K_L0:skim',
        'K_L0:loose',
        '',
        True,
        path)

# Only used for Belle via b2bii
# ?
