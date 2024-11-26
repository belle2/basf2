#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Perform code quality cppchecks for every commit to the trg package.
"""

import re
from b2test_utils import check_error_free


if __name__ == "__main__":
    ignoreme = "nofile:0:0"
    check_error_free("b2code-cppcheck", "cppcheck", "trg",
                     lambda x: re.findall(ignoreme, x) or x == "'")
