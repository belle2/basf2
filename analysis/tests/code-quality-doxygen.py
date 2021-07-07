#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Perform code doxygen checks for every commit to the analysis package.
Eventually these checks can be included as git hooks.
"""

import re
from b2test_utils import check_error_free

if __name__ == "__main__":
    # run the check ignoring OrcaKinFit errors
    ignoreme = 'Belle2::OrcaKinFit'
    check_error_free("b2code-doxygen-warnings", "doxygen", "analysis",
                     lambda x: re.findall(ignoreme, x) or x == "'")
