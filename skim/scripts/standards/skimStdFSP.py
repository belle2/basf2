#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
#
# Author(s): F.Tenchini (francesco.tenchini@desy.de) et al.
#
########################################################

from basf2 import *
from modularAnalysis import *
from stdKlongs import stdKlongs

# KL


def stdSkimKL0(path=analysis_main):
    stdKlongs('loose', path)
    cutAndCopyList(
        'K_L0:skim',
        'K_L0:loose',
        '',
        True,
        path)
