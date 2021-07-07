#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################################
#
# This steering file creates a simple VTX with single sensor in +X to mimik
# a testbeam geometry. A particle gun shoots electron at fixed angles.
#
##############################################################################

import os
import basf2 as b2
import simulation as sim
import reconstruction as rec

from ROOT import Belle2

import ROOT
ROOT.gROOT.SetBatch(0)

# register necessary modules
main = b2.create_path()

# list of parts obstructing view of on cvd volume
other_parts = [
    'COIL',
    'STR',
    'ServiceGapsMaterial',
    'BeamPipe',
    'Cryostat',
    'FarBeamLine',
    'HeavyMetalShield',
    'VXDService',
    'MagneticField']


main.add_module('EventInfoSetter')

main.add_module('ParticleGun', pdgCodes=[11],
                momentumGeneration="fixed", momentumParams=[5.2],
                thetaGeneration="fixed", thetaParams=[90.],
                phiGeneration="fixed", phiParams=[0.],
                xVertexParams=[-0.02, 0.02],
                yVertexParams=[0.28, 0.32],
                zVertexParams=[0.95, 0.105]
                )
main.add_module('Gearbox')
# remove PXD+SVD and add VTX
# remove other detectors and structures for better view on VXT (optional)
main.add_module('Geometry', excludedComponents=['PXD', 'SVD', 'CDC', 'ECL', 'ARICH', 'TOP', 'KLM'] + other_parts,
                additionalComponents=['VTX-CMOS-testbeam-2020-03-12'],
                useDB=False)

# Add simulation modules
sim.add_simulation(main, components=['VTX'], forceSetPXDDataReduction=True, usePXDDataReduction=False, useVTX=True)

# Add mc reconstruction
rec.add_mc_reconstruction(main, components=['VTX'], pruneTracks=False, useVTX=True)


display = b2.register_module('Display')
display.param('fullGeometry', True)
display.param('hideVolumes', ['logicalCDC',
                              'ARICH.masterVolume',
                              'BKLM.EnvelopeLogical',
                              'Endcap_1',
                              'Endcap_2'])
display.param('deleteVolumes', [
    # 'BKLM\\.EnvelopeLogical',
    # '^Endcap_1.*',
    # '^Endcap_2.*',
    # --- BKLM ---------------------------------------------------
    '#BKLM\\.F',       # Note the leading # (only delete children)
    '#BKLM\\.B',       # Note the leading # (only delete children)
    # --- EKLM ---------------------------------------------------
    '#^Layer_.*',      # Note the leading # (only delete children)
    '#^ShieldLayer.*',  # Note the leading # (only delete children)
    # --- CDC ----------------------------------------------------
    '#^logicalCDC_0$',  # delete children + hide this (see above)
    # --- TOP ----------------------------------------------------
    '#^TOPEnvelopeModule.*',
    # --- ARICH --------------------------------------------------
    '#ARICH.*',        # delete children + hide this (see above)
])
main.add_module(display)
b2.process(main)
print(b2.statistics)
