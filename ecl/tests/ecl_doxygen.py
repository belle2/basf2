#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Perform code doxygen checks for every commit to the ecl package.
Eventually these checks can be included as git hooks.
"""

import re
from b2test_utils import check_error_free

if __name__ == "__main__":
    #: ignore a few cases until they are fixed
    ignoreDiscreteCosineTransform = 'DiscreteCosineTransform'
    ignoreEclConfigurationPure = 'EclConfigurationPure'

    check_error_free("b2code-doxygen-warnings", "doxygen", "ecl",
                     lambda x:
                     re.findall(ignoreDiscreteCosineTransform, x) or
                     re.findall(ignoreEclConfigurationPure, x) or
                     x == "'")
