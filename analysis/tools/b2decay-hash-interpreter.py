#!/usr/bin/env python3

import os
import sys
import decayHash

if __name__ == '__main__':
    if len(sys.argv) < 4:
        print(f'Usage {os.path.basename(__file__)} hashmap.root decayHash decayHashExtended')

    rootfile = sys.argv[1]
    hashmap = decayHash.DecayHashMap(rootfile)
    hashmap.print_hash(float(sys.argv[2]), float(sys.argv[3]))
