#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *


def add_packers(path, components=None):
    """
    This function adds the raw data packer modules to a path.
    """

    # PXD
    if components == None or 'PXD' in components:
        pxdpacker = register_module('PXDPacker')
        pxdpacker.param('dhh_to_dhhc', [
            [
                0,
                2,
                4,
                34,
                36,
                38,
                ],
            [
                1,
                6,
                8,
                40,
                42,
                44,
                ],
            [
                2,
                10,
                12,
                46,
                48,
                50,
                ],
            [
                3,
                14,
                16,
                52,
                54,
                56,
                ],
            [
                4,
                3,
                5,
                35,
                37,
                39,
                ],
            [
                5,
                7,
                9,
                41,
                43,
                45,
                ],
            [
                6,
                11,
                13,
                47,
                49,
                51,
                ],
            [
                7,
                15,
                17,
                53,
                55,
                57,
                ],
            ])

        path.add_module(pxdpacker)

    # BPID
    if components == None or 'BPID' in components:
        bpidpacker = register_module('TOPPacker')
        path.add_module(bpidpacker)


def add_unpackers(path, components=None):
    """
    This function adds the raw data unpacker modules to a path.
    """

    # PXD
    if components == None or 'PXD' in components:
        pxdunpacker = register_module('PXDUnpacker')
        pxdunpacker.param('HeaderEndianSwap', True)
        path.add_module(pxdunpacker)

        pxdhitsorter = register_module('PXDRawHitSorter')
        pxdhitsorter.param('mergeFrames', False)
        path.add_module(pxdhitsorter)

    # BPID
    if components == None or 'BPID' in components:
        bpidunpacker = register_module('TOPUnpacker')
        path.add_module(bpidunpacker)


def add_raw_output(path, filename='raw.root', additionalBranches=[]):
    """
    This function adds an output module for raw data to a path.
    """

    output = register_module('RootOutput')
    output.param('outputFileName', filename)
    branches = ['RawPXDs']
    branches += ['RawBPIDs']
    branches += additionalBranches
    output.param('branchNames', branches)
    path.add_module(output)


