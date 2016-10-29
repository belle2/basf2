#!/usr/bin/env python3

import sys
import decayHash

if __name__ == '__main__':
    if len(sys.argv) < 4:
        print('Usage {name} hashmap.root decayHash decayHashExtended'.format(sys.argv[0]))

    rootfile = sys.argv[1]
    hash2string = decayHash.parse_rootfile(rootfile)

    decayHash.print_hash(hash2string[decayHash.decayHashFloatToInt(float(sys.argv[2]), float(sys.argv[3]))])
