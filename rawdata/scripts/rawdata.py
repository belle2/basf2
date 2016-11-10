#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2


def add_packers(path, components=None):
    """
    This function adds the raw data packer modules to a path.
    """

    # PXD
    if components is None or 'PXD' in components:
        pxdpacker = register_module('PXDPacker')
        pxdpacker.param('dhe_to_dhc', [
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

    # SVD
    if components is None or 'SVD' in components:
        svdpacker = register_module('SVDPacker')
        path.add_module(svdpacker)

    # CDC
    if components is None or 'CDC' in components:
        cdcpacker = register_module('CDCPacker')
        cdcpacker.param('xmlMapFileName', Belle2.FileSystem.findFile("data/cdc/ch_map.dat"))
        path.add_module(cdcpacker)

    # ECL
    if components is None or 'ECL' in components:
        eclpacker = register_module('ECLPacker')
        path.add_module(eclpacker)

    # TOP
    if components is None or 'TOP' in components:
        toppacker = register_module('TOPPacker')
        path.add_module(toppacker)

    # ARICH
    if components is None or 'ARICH' in components:
        arichpacker = register_module('ARICHPacker')
        path.add_module(arichpacker)

    # BKLM
    if components is None or 'BKLM' in components:
        bklmpacker = register_module('BKLMRawPacker')
        path.add_module(bklmpacker)


def add_unpackers(path, components=None):
    """
    This function adds the raw data unpacker modules to a path.
    """

    # PXD
    if components is None or 'PXD' in components:
        pxdunpacker = register_module('PXDUnpacker')
        pxdunpacker.param('HeaderEndianSwap', True)
        path.add_module(pxdunpacker)

        pxdhitsorter = register_module('PXDRawHitSorter')
        pxdhitsorter.param('mergeFrames', False)
        path.add_module(pxdhitsorter)

    # SVD
    if components is None or 'SVD' in components:
        svdunpacker = register_module('SVDUnpacker')
        path.add_module(svdunpacker)

    # CDC
    if components is None or 'CDC' in components:
        cdcunpacker = register_module('CDCUnpacker')
        cdcunpacker.param('xmlMapFileName', Belle2.FileSystem.findFile("data/cdc/ch_map.dat"))
        cdcunpacker.param('enablePrintOut', False)
        path.add_module(cdcunpacker)

    # ECL
    if components is None or 'ECL' in components:
        eclunpacker = register_module('ECLUnpacker')
        path.add_module(eclunpacker)

    # TOP
    if components is None or 'TOP' in components:
        topunpacker = register_module('TOPUnpacker')
        path.add_module(topunpacker)
        topconverter = register_module('TOPRawDigitConverter')
        topconverter.param('useSampleTimeCalibration', False)
        topconverter.param('useChannelT0Calibration', False)
        topconverter.param('useModuleT0Calibration', False)
        topconverter.param('useCommonT0Calibration', False)
        topconverter.param('subtractOffset', True)
        path.add_module(topconverter)

    # ARICH
    if components is None or 'ARICH' in components:
        arichunpacker = register_module('ARICHUnpacker')
        path.add_module(arichunpacker)

    # BKLM
    if components is None or 'BKLM' in components:
        bklmunpacker = register_module('BKLMUnpacker')
        path.add_module(bklmunpacker)


def add_raw_output(path, filename='raw.root', additionalBranches=[]):
    """
    This function adds an output module for raw data to a path.
    """

    output = register_module('RootOutput')
    output.param('outputFileName', filename)
    branches = ['RawPXDs', 'RawSVDs', 'RawTOPs', 'RawARICHs', 'RawKLMs']
    branches += additionalBranches
    output.param('branchNames', branches)
    path.add_module(output)
