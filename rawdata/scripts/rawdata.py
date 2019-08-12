#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from geometry import check_components
from ROOT import Belle2
from pxd import add_pxd_packer, add_pxd_unpacker
from svd import add_svd_packer, add_svd_unpacker
from iov_conditional import make_conditional_at


def add_packers(path, components=None):
    """
    This function adds the raw data packer modules to a path.
    """

    # Check components.
    check_components(components)

    # Add Gearbox or geometry to path if not already there
    if "Gearbox" not in path:
        path.add_module("Gearbox")

    if "Geometry" not in path:
        path.add_module("Geometry")

    # PXD
    if components is None or 'PXD' in components:
        add_pxd_packer(path)

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

    # Check components.
    check_components(components)

    # Add Gearbox or geometry to path if not already there
    if "Gearbox" not in path:
        path.add_module("Gearbox")

    if "Geometry" not in path:
        path.add_module("Geometry")

    # PXD
    if components is None or 'PXD' in components:
        add_pxd_unpacker(path)

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
        eclunpacker.param("storeTrigTime", True)
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

    # KLM
    if components is None or 'BKLM' in components or 'EKLM' in components:
        klmunpacker = register_module('KLMUnpacker')
        path.add_module(klmunpacker)

    # TRG
    if components is None or 'TRG' in components:

        trggdlunpacker = register_module('TRGGDLUnpacker')
        path.add_module(trggdlunpacker)
        trggdlsummary = register_module('TRGGDLSummary')
        path.add_module(trggdlsummary)
        trgeclunpacker = register_module('TRGECLUnpacker')
        path.add_module(trgeclunpacker)
        trggrlunpacker = register_module('TRGGRLUnpacker')
        path.add_module(trggrlunpacker)

        nmod_tsf = [0, 1, 2, 3, 4, 5, 6]
        for mod_tsf in nmod_tsf:
            path.add_module('TRGCDCTSFUnpacker', TSFMOD=mod_tsf)

        nmod_t3d = [0, 1, 2, 3]
        for mod_t3d in nmod_t3d:
            path.add_module('TRGCDCT3DUnpacker', T3DMOD=mod_t3d)

        # unpacker for neurotrigger
        neurounpacker = register_module('CDCTriggerUnpacker')
        neurounpacker.param('headerSize', 3)
        # unpack the data from the 2D tracker and save its Bitstream
        neurounpacker.param('unpackTracker2D', False)
        # make CDCTriggerTrack and CDCTriggerSegmentHit objects from the 2D output
        neurounpacker.param('decode2DFinderTrack', True)
        # make CDCTriggerSegmentHit objects from the 2D input
        neurounpacker.param('decode2DFinderInput', True)
        neurounpacker.param('2DNodeId', [
            [0x11000001, 0],
            [0x11000001, 1],
            [0x11000002, 0],
            [0x11000002, 1],
        ])
        neurounpacker.param('NeuroNodeId', [
            [0x11000005, 0],
            [0x11000005, 1],
            [0x11000006, 0],
            [0x11000006, 1],
        ])
        neurounpacker.param('unpackNeuro', True)
        neurounpacker.param('decodeNeuro', True)
        neurounpacker.param('delayNNOutput', [9, 9, 9, 9])
        neurounpacker.param('delayNNSelect', [4, 4, 4, 4])
        path.add_module(neurounpacker)


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
