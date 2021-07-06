#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# @cond SUPPRESS_DOXYGEN

"""
<header>
<contact>Kilian Lieret, Kilian.Lieret@campus.lmu.de</contact>


<description>
This file will exit with an exit code != 0 to probe the
validation correctly detects and reports this.
</description>
</header>
"""


import sys

if __name__ == "__main__":
    print("This script will exit with code 20 on purpose")

    sys.exit(20)

# @endcond
