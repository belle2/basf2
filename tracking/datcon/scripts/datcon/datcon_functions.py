#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2


def add_datcon(path):
    """
    Convenience function to add the FPGA version of DATCON to the path.
    : param path: The path to add the DATCONFPGA module to.
    """

    path.add_module('DATCON',
                    uSideVerticalHoughSpaceSize=100000000,
                    uSideNAngleSectors=128,
                    uSideNVerticalSectors=128,
                    uSideMinimumHSClusterSize=5,
                    uSideMaximumHSClusterSize=50,
                    vSideIsUClusterizer=False,    # required, as default is True for u side
                    vSideIsUFinder=False,         # required, as default is True for u side
                    vSideNAngleSectors=128,
                    vSideNVerticalSectors=64,
                    vSideMinimumHSClusterSize=5,
                    vSideMaximumHSClusterSize=50,
                    vSideMinimumX=-75. / 180. * 3.14159265359,  # corresponds to  +15° for theta (default values are for u side)
                    vSideMaximumX=65. / 180. * 3.14159265359,  # corresponds to +155° for theta (default values are for u side)
                    vSideVerticalHoughSpaceSize=20000000,
                    createPXDIntercepts=False,
                    extrapolationPhiCutL1=1.,
                    extrapolationPhiCutL2=1.,
                    uROIsizeL1=80,
                    uROIsizeL2=60,
                    vROIsizeL1=80,
                    vROIsizeL2=60)
