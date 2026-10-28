#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Perform code quality cppchecks for every commit to the mva package.
"""

import re
from b2test_utils import check_error_free

if __name__ == "__main__":
    # check_error_free skips all the lines identified by findall, so here we skip all the lines
    # that do no not contain the name of the tested package.
    ignoreme = r"^((?!mva\/).)*$"
    check_error_free("b2code-cppcheck", "cppcheck", "mva",
                     lambda x: re.findall(ignoreme, x) or x == "'")
