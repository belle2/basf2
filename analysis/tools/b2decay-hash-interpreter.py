#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import sys

if __name__ == '__main__':
    if len(sys.argv) < 4:
        print(f'Usage {os.path.basename(__file__)} hashmap.root decayHash decayHashExtended')

    # lazy import of decayHash for lazy import of ROOT, otherwise it seems that __file__ is root in the main
    import decayHash
    rootfile = sys.argv[1]
    hashmap = decayHash.DecayHashMap(rootfile)
    hashmap.print_hash(float(sys.argv[2]), float(sys.argv[3]))
