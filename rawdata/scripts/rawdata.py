#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
from svd import add_svd_packer, add_svd_unpacker
from framework.iov_conditional import make_conditional_at


def add_packers(path, components=None):
    """
    This function adds the raw data packer modules to a path.
    """

    # Add Gearbox or geometry to path if not already there
    if "Gearbox" not in path:
        path.add_module("Gearbox")

    if "Geometry" not in path:
        path.add_module("Geometry")

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
        add_svd_packer(path)

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

    # EKLM
    if components is None or 'EKLM' in components:
        eklmpacker = register_module('EKLMRawPacker')
        path.add_module(eklmpacker)


def add_unpackers(path, components=None):
    """
    This function adds the raw data unpacker modules to a path.
    """

    # Add Gearbox or geometry to path if not already there
    if "Gearbox" not in path:
        path.add_module("Gearbox")

    if "Geometry" not in path:
        path.add_module("Geometry")

    # PXD
    if components is None or 'PXD' in components:
        pxdunpacker = register_module('PXDUnpacker')
        path.add_module(pxdunpacker)

        pxderrorcheck = register_module('PXDPostErrorChecker')
        path.add_module(pxderrorcheck)

        pxdhitsorter = register_module('PXDRawHitSorter')
        path.add_module(pxdhitsorter)
        path.add_module('ActivatePXDPixelMasker')

    # SVD
    if components is None or 'SVD' in components:
        add_svd_unpacker(path)

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
        path.add_module(topconverter)

    # ARICH
    if components is None or 'ARICH' in components:
        arichunpacker = register_module('ARICHUnpacker')
        path.add_module(arichunpacker)

    # BKLM
    if components is None or 'BKLM' in components:
        bklmunpacker = register_module('BKLMUnpacker')
        path.add_module(bklmunpacker)

    # EKLM
    if components is None or 'EKLM' in components:
        eklmunpacker = register_module('EKLMUnpacker')
        path.add_module(eklmunpacker)

    # TRG
    if components is None or 'TRG' in components:
        gdl_unpack_path = basf2.create_path()
        gdl_unpack_path.add_module('TRGGDLUnpacker')
        gdl_unpack_path.add_module('TRGGDLSummary')

        gdl_no_unpack_path = basf2.create_path()

        # The GDL unpacker currently does not support runs before experiment 3, run 677
        # Therefore, we only unpack runs after that and also not for MC, because there is no
        # packer for the GDL content
        # We will use the new unpacker and I will create a steering file conditional path so
        # only runs => e3r677 will be unpacked. For runs before that, no trigger bits will be available.
        # Hideyuki Nakazawa will provide two modules of the TRG unpacker module. One for runs
        # before e3r677 and one for runs after that.
        make_conditional_at(path, iov_list=[(3, 677, 3, -1), (4, 0, 4, -1)],
                            path_when_in_iov=gdl_unpack_path,
                            path_when_not_in_iov=gdl_no_unpack_path)

        trgeclunpacker = register_module('TRGECLUnpacker')
        path.add_module(trgeclunpacker)


def add_raw_output(path, filename='raw.root', additionalBranches=[]):
    """
    This function adds an output module for raw data to a path.
    """

    output = register_module('RootOutput')
    output.param('outputFileName', filename)
    branches = ['RawPXDs', 'RawSVDs', 'RawCDCs', 'RawTOPs', 'RawARICHs', 'RawECLs', 'RawKLMs']
    branches += additionalBranches
    output.param('branchNames', branches)
    path.add_module(output)


def add_raw_seqoutput(path, filename='raw.sroot', additionalObjects=[], fileNameIsPattern=False):
    """
    This function adds an seqroot output module for raw data to a path.

    :param bool fileNameIsPattern: If true the filename needs to be a printf pattern with a placeholder for the
    filenumber starting at 0, for example "raw-f%06d.root"
    """

    output = register_module('SeqRootOutput')
    output.param('outputFileName', filename)
    output.param('fileNameIsPattern', fileNameIsPattern)
    objects = ['EventMetaData', 'RawPXDs', 'RawSVDs', 'RawCDCs', 'RawTOPs', 'RawARICHs', 'RawECLs', 'RawKLMs']
    objects += additionalObjects
    output.param('saveObjs', objects)
    path.add_module(output)
