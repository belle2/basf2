#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
from tracking import add_tracking_for_PXDDataReduction_simulation


def check_simulation(path):
    """Check if the minimum number of modules required for simulation are in
    the path and in the correct order"""
    required = ['Gearbox', 'Geometry', 'FullSim']
    found = []

    # make a list of all required modules in the path
    for module in path.modules():
        module_type = module.type()
        # if required add to list of found modules
        if module_type in required:
            # unless it is already in there
            if module_type in found:
                B2ERROR("Duplicate module in path: %s" % module_type)
            else:
                found.append(module.type())

    if len(required) != len(found):
        # Apparently at least one module is missing
        for r in required:
            if r not in found:
                B2ERROR("No '%s' module found but needed for simulation" % r)
    # We have all modules but do they have the correct order?
    elif required != found:
        B2ERROR("Simulation modules in wrong order. Should be '%s' but is '%s'"
                % (", ".join(required), ", ".join(found)))


def add_PXDDataReduction(path, components, use_vxdtf2=False):

    pxd_unfiltered_digits = 'pxd_unfiltered_digits'
    pxd_digitizer = register_module('PXDDigitizer')
    pxd_digitizer.param('Digits', pxd_unfiltered_digits)
    path.add_module(pxd_digitizer)

    # SVD tracking

    svd_reco_tracks = '__ROIsvdRecoTracks'

    add_tracking_for_PXDDataReduction_simulation(path, components, use_vxdtf2)

    pxdDataRed = register_module('PXDDataReduction')
    param_pxdDataRed = {
        'recoTrackListName': svd_reco_tracks,
        'PXDInterceptListName': 'PXDIntercepts',
        'ROIListName': 'ROIs',
        'tolerancePhi': 0.15,
        'toleranceZ': 0.5,
        'sigmaSystU': 0.1,
        'sigmaSystV': 0.1,
        'numSigmaTotU': 10,
        'numSigmaTotV': 10,
        'maxWidthU': 10,
        'maxWidthV': 10,
    }
    pxdDataRed.param(param_pxdDataRed)
    path.add_module(pxdDataRed)

    # Filtering of PXDDigits
    pxd_digifilter = register_module('PXDdigiFilter')
    pxd_digifilter.param('ROIidsName', 'ROIs')
    pxd_digifilter.param('PXDDigitsName', pxd_unfiltered_digits)
    pxd_digifilter.param('PXDDigitsInsideROIName', 'PXDDigits')
    path.add_module(pxd_digifilter)


def add_simulation(
        path,
        components=None,
        bkgfiles=None,
        bkgcomponents=None,
        bkgscale=1.0,
        usePXDDataReduction=True,
        use_vxdtf2=False):
    """
    This function adds the standard simulation modules to a path.
    """

    # background mixing
    if bkgfiles:
        bkgmixer = register_module('BeamBkgMixer')
        bkgmixer.param('backgroundFiles', bkgfiles)
        if bkgcomponents:
            bkgmixer.param('components', bkgcomponents)
        else:
            if components:
                bkgmixer.param('components', components)
        bkgmixer.param('overallScaleFactor', bkgscale)
        path.add_module(bkgmixer)

    # geometry parameter database
    if 'Gearbox' not in path:
        gearbox = register_module('Gearbox')
        path.add_module(gearbox)

    # detector geometry
    if 'Geometry' not in path:
        geometry = register_module('Geometry')
        if components:
            geometry.param('components', components)
        path.add_module(geometry)

    # detector simulation
    if 'FullSim' not in path:
        g4sim = register_module('FullSim')
        path.add_module(g4sim)

    check_simulation(path)

    # no checks are performed for BeamBkgMixer and the Digitizers as they are
    # not necessary for running simulation jobs and it should be possible to
    # have them in the path more than once

    # SVD digitization
    if components is None or 'SVD' in components:
        svd_digitizer = register_module('SVDDigitizer')
        path.add_module(svd_digitizer)
        svd_clusterizer = register_module('SVDClusterizer')
        path.add_module(svd_clusterizer)

    # CDC digitization
    if components is None or 'CDC' in components:
        cdc_digitizer = register_module('CDCDigitizer')
        path.add_module(cdc_digitizer)

    # PXD digitization
    if components is None or 'PXD' in components:
        if usePXDDataReduction:
            add_PXDDataReduction(path, components, use_vxdtf2)
        else:
            pxd_digitizer = register_module('PXDDigitizer')
            path.add_module(pxd_digitizer)
        pxd_clusterizer = register_module('PXDClusterizer')
        path.add_module(pxd_clusterizer)

    # TOP digitization
    if components is None or 'TOP' in components:
        top_digitizer = register_module('TOPDigitizer')
        path.add_module(top_digitizer)

    # ARICH digitization
    if components is None or 'ARICH' in components:
        arich_digitizer = register_module('ARICHDigitizer')
        path.add_module(arich_digitizer)

    # ECL digitization
    if components is None or 'ECL' in components:
        ecl_digitizer = register_module('ECLDigitizer')
        if bkgfiles is not None:
            ecl_digitizer.param('Background', 1)
        path.add_module(ecl_digitizer)

    # BKLM digitization
    if components is None or 'BKLM' in components:
        bklm_digitizer = register_module('BKLMDigitizer')
        path.add_module(bklm_digitizer)

    # EKLM digitization
    if components is None or 'EKLM' in components:
        eklm_digitizer = register_module('EKLMDigitizer')
        path.add_module(eklm_digitizer)


def add_cosmics_simulation(path, components=None, globalBoxSize=["20", "20", "9"], acceptBox=[8, 8, 8],
                           keepBox=[8, 8, 8], usePXDDataReduction=False, use_vxdtf2=False):
    """
    This function adds the cosmic simulation modules to a path.
    CRY generator is used to generate cosmic rays.
    :param path: Add the modules to this path.
    :param components: list of geometry components to include simulation for,
           or None for all components.
    :param globalBoxSize: sets Global length, width and height.
    :param acceptBox: sets the size of the accept box. As a default it is
           set to 8.0 m as Belle2 detector size.
    :param keepBox: sets the size of the keep box (keep box >= accept box).
    """

    # geometry parameter database
    if 'Gearbox' not in path:
        gearbox = register_module('Gearbox', override=[
            ("/Global/length", globalBoxSize[0], "m"),
            ("/Global/width", globalBoxSize[1], "m"),
            ("/Global/height", globalBoxSize[2], "m")]
        )
        path.add_module(gearbox)

    # detector geometry
    if 'Geometry' not in path:
        geometry = register_module('Geometry')
        if components:
            geometry.param('components', components)
        path.add_module(geometry)

    # registering the CRY module and setting up its params
    if 'CRYInput' not in path:
        cry = register_module('CRYInput')

        # cosmic data input
        cry.param('CosmicDataDir', Belle2.FileSystem.findFile('data/generators/modules/cryinput/'))

        # user input file
        cry.param('SetupFile', Belle2.FileSystem.findFile('simulation/scripts/cry.setup'))

        # acceptance half-lengths - at least one particle has to enter that box to use that event
        cry.param('acceptLength', acceptBox[0])
        cry.param('acceptWidth', acceptBox[1])
        cry.param('acceptHeight', acceptBox[2])
        cry.param('maxTrials', 100000)

        # keep half-lengths - all particles that do not enter the box are removed (keep box >= accept box)
        # default was 6.0
        cry.param('keepLength', keepBox[0])
        cry.param('keepWidth', keepBox[1])
        cry.param('keepHeight', keepBox[2])

        # minimal kinetic energy - all particles below that energy are ignored
        cry.param('kineticEnergyThreshold', 0.01)
        path.add_module(cry)

    # detector simulation
    if 'FullSim' not in path:
        g4sim = register_module('FullSim')
        path.add_module(g4sim)

    check_simulation(path)

    # no checks are performed for BeamBkgMixer and the Digitizers as they are
    # not necessary for running simulation jobs and it should be possible to
    # have them in the path more than once

    # SVD digitization
    if components is None or 'SVD' in components:
        svd_digitizer = register_module('SVDDigitizer')
        path.add_module(svd_digitizer)
        svd_clusterizer = register_module('SVDClusterizer')
        path.add_module(svd_clusterizer)

    # CDC digitization
    if components is None or 'CDC' in components:
        cdc_digitizer = register_module('CDCDigitizer')
        path.add_module(cdc_digitizer)

    # PXD digitization
    if components is None or 'PXD' in components:
        if usePXDDataReduction:
            add_PXDDataReduction(path, components, use_vxdtf2)
        else:
            pxd_digitizer = register_module('PXDDigitizer')
            path.add_module(pxd_digitizer)
        pxd_clusterizer = register_module('PXDClusterizer')
        path.add_module(pxd_clusterizer)

    # TOP digitization
    if components is None or 'TOP' in components:
        top_digitizer = register_module('TOPDigitizer')
        path.add_module(top_digitizer)

    # ARICH digitization
    if components is None or 'ARICH' in components:
        arich_digitizer = register_module('ARICHDigitizer')
        path.add_module(arich_digitizer)

    # ECL digitization
    if components is None or 'ECL' in components:
        ecl_digitizer = register_module('ECLDigitizer')
        path.add_module(ecl_digitizer)

    # BKLM digitization
    if components is None or 'BKLM' in components:
        bklm_digitizer = register_module('BKLMDigitizer')
        path.add_module(bklm_digitizer)

    # EKLM digitization
    if components is None or 'EKLM' in components:
        eklm_digitizer = register_module('EKLMDigitizer')
        path.add_module(eklm_digitizer)
