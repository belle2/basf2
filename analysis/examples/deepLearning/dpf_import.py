#!/usr/bin/env python

import struct
import collections

Header = collections.namedtuple('Header', 'x y z px py pz prob mcerrors isSignal isRoeB0 nTracks nECLCluster nKLMCluster')
header_format = '=7f6i'
header_size = struct.calcsize(header_format)
header_entry = struct.Struct(header_format)

Valid = collections.namedtuple('Valid', 'flag')
valid_format = '=i'
valid_size = struct.calcsize(valid_format)
valid_entry = struct.Struct(valid_format)

Track = collections.namedtuple('Track', 'charge x y z px py pz prob eid muid kid prid')
track_format = '=i11f'
track_size = struct.calcsize(track_format)
track_entry = struct.Struct(track_format)

ECL = collections.namedtuple('ECLCluster', 'x y z E time ratio lat track')
ecl_format = '=7fb'
ecl_size = struct.calcsize(ecl_format)
ecl_entry = struct.Struct(ecl_format)

KLM = collections.namedtuple('KLMCluster', 'x y z E time layers eclCluster track')
klm_format = '=6f2b'
klm_size = struct.calcsize(klm_format)
klm_entry = struct.Struct(klm_format)

Event = collections.namedtuple('Event', 'header tracks ecl_clusters klm_clusters')


def read(f):
    raw_header = f.read(header_size)
    if len(raw_header) != header_size:
        return None
    header = Header._make(header_entry.unpack_from(raw_header))

    tracks = [Track._make(track_entry.unpack_from(f.read(track_size))) for i in range(header.nTracks)
              if Valid._make(valid_entry.unpack_from(f.read(valid_size))).flag == 1]

    ecls = [ECL._make(ecl_entry.unpack_from(f.read(ecl_size))) for i in range(header.nECLCluster)
            if Valid._make(valid_entry.unpack_from(f.read(valid_size))).flag == 1]

    klms = [KLM._make(klm_entry.unpack_from(f.read(klm_size))) for i in range(header.nKLMCluster)
            if Valid._make(valid_entry.unpack_from(f.read(valid_size))).flag == 1]

    header = Header._make(header[:-3] + (len(tracks), len(ecls), len(klms)))

    return Event._make((header, tracks, ecls, klms))
