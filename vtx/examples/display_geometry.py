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
# This steering file displays the Belle II upgrade detector geometry and perfoms the
# simulation and standard reconstruction.
#
# Usage: basf2 display_geometry.py -- --vtx=VTX-CMOS-5layer-Discs
#
##############################################################################

import argparse
import os
import basf2 as b2
import simulation as sim
import reconstruction as rec

from ROOT import Belle2

import ROOT
ROOT.gROOT.SetBatch(0)

ap = argparse.ArgumentParser()
ap.add_argument("--vtx", default="VTX-CMOS-5layer-Discs", help="VTX geometry variant from xml")
args = vars(ap.parse_args())

print("INFO: Display for " + args['vtx'])

# register necessary modules
main = b2.create_path()

# list of parts obstructing view of on cvd volume
other_parts = ['COIL', 'STR', 'ServiceGapsMaterial', 'BeamPipe', 'Cryostat', 'FarBeamLine', 'HeavyMetalShield', 'VXDService']


main.add_module('EventInfoSetter')
main.add_module('EvtGenInput')
main.add_module('Gearbox')
# remove PXD+SVD and add VTX
# remove other detectors and structures for better view on VXT (optional)
main.add_module('Geometry', excludedComponents=['PXD', 'SVD', 'CDC', 'ECL', 'ARICH', 'TOP', 'KLM'] + other_parts,
                additionalComponents=[args['vtx']],
                useDB=False)

# Add simulation modules
sim.add_simulation(main, components=['VTX'], useVTX=True)

# Add mc reconstruction
# FIXME: does currently not work if CDC is not in the Geometry, simulation and reconstruction,
# but for just showing the geometry the MC reconstruction is not needed.
# rec.add_mc_reconstruction(main, components=['VTX'], pruneTracks=False, useVTX=True, useVTXClusterShapes=False)

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
