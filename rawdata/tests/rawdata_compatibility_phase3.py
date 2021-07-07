#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from rawdata_compatibility_base import test_raw

if __name__ == "__main__":
    # use the None as GT so the default global tag will be used for Monte Carlo generated files
    test_raw("phase3", "mc", None)
