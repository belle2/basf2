#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
from geometry import check_components
from pxd import add_pxd_packer, add_pxd_unpacker
from svd import add_svd_packer, add_svd_unpacker
from neurotrigger import add_neuro_2d_unpackers


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
        cdcpacker = b2.register_module('CDCPacker')
        path.add_module(cdcpacker)

    # ECL
    if components is None or 'ECL' in components:
        eclpacker = b2.register_module('ECLPacker')
        path.add_module(eclpacker)

    # TOP
    if components is None or 'TOP' in components:
        toppacker = b2.register_module('TOPPacker')
        path.add_module(toppacker)

    # ARICH
    if components is None or 'ARICH' in components:
        arichpacker = b2.register_module('ARICHPacker')
        path.add_module(arichpacker)

    # KLM
    if components is None or 'KLM' in components:
        klmpacker = b2.register_module('KLMPacker')
        path.add_module(klmpacker)


def add_unpackers(path, components=None, writeKLMDigitRaws=False, addTOPRelations=False):
    """
    This function adds the raw data unpacker modules to a path.

    :param components: list of geometry components to include reconstruction for, or None for all components.
    :param writeKLMDigitRaws: flag for creating the KLMDigitRaw object and storing it in the datastore. The KLMDQM
        module needs it for filling some histograms.
    :param addTOPRelations: flag for creating relations in TOPUnpacker and TOPRawDigitConverter
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
        cdcunpacker = b2.register_module('CDCUnpacker')
        cdcunpacker.param('enableStoreCDCRawHit', True)
        cdcunpacker.param('enablePrintOut', False)
        path.add_module(cdcunpacker)

    # ECL
    if components is None or 'ECL' in components:
        eclunpacker = b2.register_module('ECLUnpacker')
        eclunpacker.param("storeTrigTime", True)
        path.add_module(eclunpacker)

    # TOP
    if components is None or 'TOP' in components:
        topunpacker = b2.register_module('TOPUnpacker')
        topunpacker.param('addRelations', addTOPRelations)
        path.add_module(topunpacker)
        topconverter = b2.register_module('TOPRawDigitConverter')
        topconverter.param('addRelations', addTOPRelations)
        path.add_module(topconverter)

    # ARICH
    if components is None or 'ARICH' in components:
        arichunpacker = b2.register_module('ARICHUnpacker')
        path.add_module(arichunpacker)

    # KLM
    if components is None or 'KLM' in components:
        klmunpacker = b2.register_module('KLMUnpacker')
        klmunpacker.param('WriteDigitRaws', writeKLMDigitRaws)
        path.add_module(klmunpacker)

    # TRG
    if components is None or 'TRG' in components:

        trggdlunpacker = b2.register_module('TRGGDLUnpacker')
        path.add_module(trggdlunpacker)
        trggdlsummary = b2.register_module('TRGGDLSummary')
        path.add_module(trggdlsummary)
        trgeclunpacker = b2.register_module('TRGECLUnpacker')
        path.add_module(trgeclunpacker)
        trggrlunpacker = b2.register_module('TRGGRLUnpacker')
        path.add_module(trggrlunpacker)
        trgtopunpacker = b2.register_module('TRGTOPUnpacker')
        path.add_module(trgtopunpacker)

        nmod_tsf = [0, 1, 2, 3, 4, 5, 6]
        for mod_tsf in nmod_tsf:
            path.add_module('TRGCDCTSFUnpacker', TSFMOD=mod_tsf)

        nmod_t3d = [0, 1, 2, 3]
        for mod_t3d in nmod_t3d:
            path.add_module('TRGCDCT3DUnpacker', T3DMOD=mod_t3d)

        # unpacker for neurotrigger
        add_neuro_2d_unpackers(path)


def add_raw_output(path, filename='raw.root', additionalBranches=None):
    """
    This function adds an output module for raw data to a path.
    """
    if additionalBranches is None:
        additionalBranches = []
    output = b2.register_module('RootOutput')
    output.param('outputFileName', filename)
    branches = ['RawPXDs', 'RawSVDs', 'RawCDCs', 'RawTOPs', 'RawARICHs', 'RawECLs', 'RawKLMs']
    branches += additionalBranches
    output.param('branchNames', branches)
    path.add_module(output)


def add_raw_seqoutput(path, filename='raw.sroot', additionalObjects=None, fileNameIsPattern=False):
    """
    This function adds an seqroot output module for raw data to a path.

    :param bool fileNameIsPattern: If true the filename needs to be a printf pattern with a placeholder for the
    filenumber starting at 0, for example "raw-f%06d.root"
    """
    if additionalObjects is None:
        additionalObjects = []
    output = b2.register_module('SeqRootOutput')
    output.param('outputFileName', filename)
    output.param('fileNameIsPattern', fileNameIsPattern)
    objects = ['EventMetaData', 'RawPXDs', 'RawSVDs', 'RawCDCs', 'RawTOPs', 'RawARICHs', 'RawECLs', 'RawKLMs']
    objects += additionalObjects
    output.param('saveObjs', objects)
    path.add_module(output)
