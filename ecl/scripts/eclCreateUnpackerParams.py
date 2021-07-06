#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import ROOT
from ROOT import Belle2

"""
Create and fill ECLChannelMap class.
Can be used for creating run-dependent ECLUnpacker parameters.
"""

FIRST_EXP = 12
FIRST_RUN = 2251
LAST_EXP = 12
LAST_RUN = 2366
DBOBJECT_NAME = 'ECLUnpackingParameters'


def getValue(crate, shaper, channel):
    """
    Edit this function according to your current use-case.
    If you are preparing ECLUnpackingParameters, see
    ECLUnpackerModule::ECLUnpack for bit definitions.
    """
    if crate == 32 and shaper == 9:
        return 7
    return 0

################################################


def main():
    """
    """
    coefs_bar = []
    coefs_fwd = []
    coefs_bwd = []

    for crate in range(1, 52 + 1):
        if crate < 37:
            sh_count = 12
        elif crate < 45:
            sh_count = 10
        else:
            sh_count = 8
        for shaper in range(1, sh_count + 1):
            for channel in range(1, 16 + 1):
                val = getValue(crate, shaper, channel)
                if crate < 37:
                    coefs_bar.append(val)
                elif crate < 45:
                    coefs_fwd.append(val)
                else:
                    coefs_bwd.append(val)

    vec_bar = ROOT.std.vector('int')()
    vec_bar += coefs_bar
    vec_fwd = ROOT.std.vector('int')()
    vec_fwd += coefs_fwd
    vec_bwd = ROOT.std.vector('int')()
    vec_bwd += coefs_bwd

    coefs = Belle2.ECLChannelMap()
    coefs.setMappingVectors(vec_bar, vec_fwd, vec_bwd)

    db = Belle2.Database.Instance()
    iov = Belle2.IntervalOfValidity(FIRST_EXP, FIRST_RUN, LAST_EXP, LAST_RUN)
    db.storeData(DBOBJECT_NAME, coefs, iov)


if __name__ == '__main__':
    main()
