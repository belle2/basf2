#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# --------------------------------------------------------------------------
# This scripts compares different versions of ECL channel maps,
# both text files and database ecl_ch_maps
#
# Return code is
#  0-identical ECL channel maps
#  1-different ECL channel maps
#  255-wrong arguments
# --------------------------------------------------------------------------

from ROOT import Belle2, TFile
from ROOT.Belle2 import ECLChannelMap
import sys

ECL_CHANNELS_IN_SHAPER = 16
ECL_BARREL_CRATES = 36
ECL_FWD_CRATES = 8

################################################


def main():
    if len(sys.argv) < 3:
        print()
        print('Usage: %s file1 file2' % sys.argv[0])
        print('  file1 Path to text file or root file with ECL channel map')
        print('  file2 Path to text file or root file with ECL channel map')
        print()
        exit(255)

    paths = sys.argv[1], sys.argv[2]
    ch_maps = []
    ch_maps.append(loadFile(paths[0]))
    ch_maps.append(loadFile(paths[1]))

    # True if there are different rows in two maps
    diff = False

    for partial_ch_maps in zip(*ch_maps):
        for row1, row2 in zip(*partial_ch_maps):
            if row1 != row2:
                diff = True
                print('file1:', row1)
                print('file2:', row2)
    if not diff:
        print()
        print('%s and %s contain identical channel maps.' % paths)
        print()
        exit(0)
    else:
        # It should be obvious from diff output that files are different,
        # so nothing is printed here.
        exit(1)

################################################

# Load ECL channel map from file (either *.txt or *.root)


def loadFile(path):
    if path.endswith('.txt'):
        return loadTextFile(path)
    elif path.endswith('.root'):
        return loadRootFile(path)
    else:
        print()
        print('Error: The script only supports *.txt and *.root. Input file: %s' % path)
        print()
        exit(255)

# Load ECL channel map from text file


def loadTextFile(path):
    map_bar = []
    map_fwd = []
    map_bwd = []
    with open(path) as f:
        for line in f.readlines():
            # Skip comments
            if line[0] == '#':
                continue
            items = [int(float(x)) for x in line.split()]
            # Skip empty lines
            if len(items) < 1:
                continue

            crate, shaper, channel, phi, theta, cell_id = items
            # Discard phiID and thetaID because they don't depend on channel map
            added_items = (crate, shaper, channel, cell_id)

            if crate <= 36:
                map_bar.append(added_items)
            elif crate <= 44:
                map_fwd.append(added_items)
            else:
                map_bwd.append(added_items)
    return map_bar, map_fwd, map_bwd

# Load ECL channel map from ROOT file


def loadRootFile(path):
    map_bar = []
    map_fwd = []
    map_bwd = []
    root_file = TFile(path, 'read')
    ecl_ch_map = root_file.Get('ECLChannelMap')

    # Based on code from ECLChannelMapper
    channel = crate = shaper = 1
    array_index = 0
    max_shapers = 12
    while crate <= 52:
        cell_id = -1
        if crate <= 36:
            cell_id = ecl_ch_map.getMappingVectorBAR()[array_index]
            map_bar.append((crate, shaper, channel, cell_id))
        elif crate <= 44:
            cell_id = ecl_ch_map.getMappingVectorFWD()[array_index]
            map_fwd.append((crate, shaper, channel, cell_id))
        else:
            cell_id = ecl_ch_map.getMappingVectorBWD()[array_index]
            map_bwd.append((crate, shaper, channel, cell_id))

        array_index += 1
        channel += 1
        if channel > ECL_CHANNELS_IN_SHAPER:
            channel = 1
            shaper += 1
            if shaper > max_shapers:
                shaper = 1
                if crate == ECL_BARREL_CRATES:
                    array_index = 0
                    max_shapers = 10
                elif crate == ECL_BARREL_CRATES + ECL_FWD_CRATES:
                    array_index = 0
                    max_shapers = 8
                crate += 1

    return map_bar, map_fwd, map_bwd

################################################


if __name__ == '__main__':
    main()
