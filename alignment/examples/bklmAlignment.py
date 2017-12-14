#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from basf2 import *
import ROOT
from ROOT import Belle2
import numpy as np


gear = register_module('Gearbox')
gear.initialize()
geom = register_module('Geometry')
geom.param('components', ['PXD', 'SVD'])
geom.initialize()

# Create the algorithm
algo = Belle2.MillepedeAlgorithm()
algo.setInputFileNames([sys.argv[1]])

# Configure Millepede
# change 'inversion' to 'diagonalization' to get lowest and highest
# eigenvectors and eigennumbers
algo.steering().command('method inversion 1 0.1')
algo.steering().command('entries 10')
algo.steering().command('chiscut 30. 6.')
algo.steering().command('outlierdownweighting 3')
algo.steering().command('dwfractioncut 0.1')
# algo.steering().command('skipemptycons 1')

# Now fix some parameters
algo.steering().command('Parameters')

# Fixing VXD params --------------------------------------
# Fix all VXDs
for vxdid in Belle2.VXD.GeoCache.getInstance().getListOfSensors():
    for ipar in range(1, 7):
        label = Belle2.GlobalLabel(vxdid, ipar)
        cmd = str(label.label()) + ' 0. -1.'
        algo.steering().command(cmd)

# Fixing CDC params --------------------------------------
# for icLayer in [item for item in list(range(0, 57)) if item not in [3, 4, 5, 6]]:

# Fix all CDC layers
for icLayer in (range(0, 57)):
    for ipar in range(1, 3):
        # Fix all for now (only layer shifts X/Y and only axial in fact)
        cmd = str(Belle2.GlobalLabel(Belle2.WireID(icLayer, 511), ipar).label()) + ' 0. -1.'
        algo.steering().command(cmd)

# Fixing BKLM params --------------------------------------
# Fix all parameters except U, V for all modules
for sector in range(1, 9):
    for layer in range(1, 16):
        for forward in [0, 1]:
            for ipar in [1, 2, 3, 4, 5, 6]:

                # For U=1 or V=2 do not fix param
                if ipar in [1, 2, 3, 4, 5, 6]:
                    continue

                bklmid = Belle2.BKLMElementID()
                bklmid.setIsForward(forward)
                bklmid.setSectorNumber(sector)
                bklmid.setLayerNumber(layer)
                label = Belle2.GlobalLabel(bklmid, ipar)
                cmd = str(label.label()) + ' 0. -1.'
                algo.steering().command(cmd)

# ---------- end of parameter fixing ----------------------


# -------------- Run the Millepede Algorithm ----------------

# Execute the algorithm over all collected data (auto-merged)
algo.execute()

# -----------------------------------------------------------


# Get the payloads into handy variables
payloads = list(algo.getPayloads())
vxd = None
cdc = None
bklm = None
for payload in payloads:
    if payload.module == 'VXDAlignment':
        vxd = payload.object.IsA().DynamicCast(Belle2.VXDAlignment().IsA(), payload.object, False)

    if payload.module == 'CDCCalibration':
        cdc = payload.object.IsA().DynamicCast(Belle2.CDCCalibration().IsA(), payload.object, False)

    if payload.module == 'BKLMAlignment':
        bklm = payload.object.IsA().DynamicCast(Belle2.BKLMAlignment().IsA(), payload.object, False)


# Profile plot for all determined parameters
bklmfile = ROOT.TFile('bklm_P123456.root', 'recreate')

profile = ROOT.TH1F(
    "profile",
    "correction & errors",
    algo.result().getNoDeterminedParameters(),
    1,
    algo.result().getNoDeterminedParameters())

# Define some branch variables
param = np.zeros(1, dtype=int)
value = np.zeros(1, dtype=float)
error = np.zeros(1, dtype=float)
layer = np.zeros(1, dtype=int)
ladder = np.zeros(1, dtype=int)
sector = np.zeros(1, dtype=int)
sensor = np.zeros(1, dtype=int)
forward = np.zeros(1, dtype=int)

# Tree with VXD data
vxdtree = ROOT.TTree('vxd', 'VXD data')
vxdtree.Branch('layer', layer, 'layer/I')
vxdtree.Branch('ladder', ladder, 'ladder/I')
vxdtree.Branch('sensor', sensor, 'sensor/I')
vxdtree.Branch('param', param, 'param/I')
vxdtree.Branch('value', value, 'value/D')
vxdtree.Branch('error', error, 'error/D')
# Tree with CDC data
cdctree = ROOT.TTree('cdc', 'CDC data')
cdctree.Branch('layer', layer, 'layer/I')
cdctree.Branch('param', param, 'param/I')
cdctree.Branch('value', value, 'value/D')
cdctree.Branch('error', error, 'error/D')
# Tree with BKLM data
bklmtree = ROOT.TTree('bklm', 'BKLM data')
bklmtree.Branch('layer', layer, 'layer/I')
bklmtree.Branch('sector', sector, 'sector/I')
bklmtree.Branch('forward', forward, 'forward/I')
bklmtree.Branch('param', param, 'param/I')
bklmtree.Branch('value', value, 'value/D')
bklmtree.Branch('error', error, 'error/D')

# Index of determined param
ibin = 0

for ipar in range(0, algo.result().getNoParameters()):
    if not algo.result().isParameterDetermined(ipar):
        continue

    ibin = ibin + 1

    label = Belle2.GlobalLabel(algo.result().getParameterLabel(ipar))
    param[0] = label.getParameterId()
    value[0] = algo.result().getParameterCorrection(ipar)
    error[0] = algo.result().getParameterError(ipar)

    if (label.isVXD()):
        layer[0] = label.getVxdID().getLayerNumber()
        ladder[0] = label.getVxdID().getLadderNumber()
        sensor[0] = label.getVxdID().getSensorNumber()
        vxdtree.Fill()

    if (label.isCDC()):
        layer[0] = label.getWireID().getICLayer()
        cdctree.Fill()

    if (label.isBKLM()):
        layer[0] = label.getBklmID().getLayerNumber()
        sector[0] = label.getBklmID().getSectorNumber()
        forward[0] = label.getBklmID().getIsForward()
        bklmtree.Fill()

    profile.SetBinContent(ibin, value[0])
    profile.SetBinError(ibin, error[0])

# Skip into interactive environment
# You can draw something in the trees or the profile
# Exit with Ctrl+D
# print('++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++')
# print(' You are now in interactive environment. You can still access the algorithm')
# print(' or the DataStore. Try e.g.:')
# print('')
# print(' >>> pval.Draw()')
# print(' >>> bklmtree.Draw("value:layer")')
# print(' >>> bklm.dump()')
# print(' >>> profile.Draw()')
# print('')
# print(' Look into this script and use TAB or python ? help to play more...')
# print(' Exit with [Ctrl] + [D] and then [Enter]')
# print('++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++')
# #import interactive
# interactive.embed()

profile.Write()
bklmtree.Write()
bklmfile.Write()

# import interactive
# interactive.embed()
