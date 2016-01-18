#!/usr/bin/env python

import sys
import struct
import collections

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Usage: {program} filename nEvents'.format(program=sys.argv[0]))
        sys.exit(1)

    Header = collections.namedtuple('Header', 'x y z px py pz prob mcerrors isSignal nTracks nECLCluster nKLMCluster')
    header_entry = struct.Struct('fffffffiiiii')

    Valid = collections.namedtuple('Valid', 'flag')
    valid_entry = struct.Struct('i')

    Track = collections.namedtuple('Track', 'x y z px py pz eid muid kid prid')
    track_entry = struct.Struct('ffffffffff')

    ECL = collections.namedtuple('ECLCluster', 'x y z E time ratio lat track')
    ecl_entry = struct.Struct('fffffffb')

    KLM = collections.namedtuple('KLMCluster', 'x y z E time layers eclCluster track')
    klm_entry = struct.Struct('ffffffbb')

    with open(sys.argv[1], 'rb') as f:
        for i in range(int(sys.argv[2])):
            header = Header._make(header_entry.unpack_from(f))
            tracks = [Track._make(track_entry.unpack_from(f)) for i in range(header.nTracks)
                      if Valid._make(valid_entry.unpack_from(f)).flag == 1]
            ecls = [ECL._make(ecl_entry.unpack_from(f)) for i in range(header.nECLCluster)
                    if Valid._make(valid_entry.unpack_from(f)).flag == 1]
            klms = [KLM._make(klm_entry.unpack_from(f)) for i in range(header.nKLMCluster)
                    if Valid._make(valid_entry.unpack_from(f)).flag == 1]

            print(header)
            print(tracks)
            print(ecls)
            print(klms)
