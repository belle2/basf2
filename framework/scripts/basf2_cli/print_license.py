#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import find_file

print('')

with open(find_file("LICENSE.md")) as license_file:
    print(license_file.read())

print("For the full text of the LGPL-3.0 License see $BELLE2_RELEASE_DIR/COPYING")
