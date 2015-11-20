#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from basf2 import *
import ROOT
from ROOT import Belle2
import numpy as np

reset_database()
use_local_database()

main = create_path()

input = register_module('RootInput')
input.param('inputFileName', 'RootOutput.root')
input.initialize()

# Create the algorithm
algo = Belle2.MillepedeAlgorithm()

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
import vxdgeometry
for layer, ladders in vxdgeometry.hierarchy.items():
    for ladder, sensors in ladders.items():
        for sensor, sensorID in sensors.items():
            vxdid = Belle2.VxdID(sensorID)
            # Fix layer 6 and all slanted SVDs (all params):
            # if vxdid.getLayerNumber() == 6 or (vxdid.getLayerNumber() > 3 and vxdid.getSensorNumber() == 1):
            label = Belle2.GlobalLabel(vxdid, 0)
            for ipar in range(1, 7):
                par_label = label.label() + ipar
                cmd = str(par_label) + ' 0. -1.'
                algo.steering().command(cmd)

# Fixing CDC params --------------------------------------
# for icLayer in [item for item in list(range(0, 57)) if item not in [3, 4, 5, 6]]:

# Fix all CDC layers
for icLayer in (range(0, 57)):
        # FIx all for now (only layer shifts X/Y and only axial in fact)
    cmd = str(Belle2.GlobalLabel(Belle2.WireID(icLayer, 511), 1).label()) + ' 0. -1.'
    algo.steering().command(cmd)
    cmd = str(Belle2.GlobalLabel(Belle2.WireID(icLayer, 511), 2).label()) + ' 0. -1.'
    algo.steering().command(cmd)

# Fixing BKLM params --------------------------------------
# Fix all parameters except U, V for all modules
for sector in range(0, 9):
    for layer in range(1, 17):
        for forward in [0, 1]:
            for ipar in [1, 2, 3, 4, 5, 6]:

                # For U=1 or V=2 do not fix param
                if ipar in [1, 2]:
                    continue

                klmid = Belle2.BKLMID()
                klmid.setIsForward(forward)
                klmid.setSectorNumber(sector)
                klmid.setLayerNumber(layer)
                label = Belle2.GlobalLabel(klmid, ipar)
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
klm = None
for payload in payloads:
    if payload.module == 'VXDAlignment':
        vxd = payload.object.IsA().DynamicCast(Belle2.VXDAlignment().IsA(), payload.object, False)

    if payload.module == 'CDCCalibration':
        cdc = payload.object.IsA().DynamicCast(Belle2.CDCCalibration().IsA(), payload.object, False)

    if payload.module == 'BKLMAlignment':
        klm = payload.object.IsA().DynamicCast(Belle2.BKLMAlignment().IsA(), payload.object, False)


# Profile plot for all determined parameters
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
klmtree = ROOT.TTree('klm', 'KLM data')
klmtree.Branch('layer', layer, 'layer/I')
klmtree.Branch('sector', sector, 'sector/I')
klmtree.Branch('forward', forward, 'forward/I')
klmtree.Branch('param', param, 'param/I')
klmtree.Branch('value', value, 'value/D')
klmtree.Branch('error', error, 'error/D')

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

    if (label.isKLM()):
        layer[0] = label.getKlmID().getLayerNumber()
        sector[0] = label.getKlmID().getSectorNumber()
        forward[0] = label.getKlmID().getIsForward()
        klmtree.Fill()

    profile.SetBinContent(ibin, value[0])
    profile.SetBinError(ibin, error[0])

# Example how to access collected data (but you need exp and run number)
chi2ndf = Belle2.PyStoreObj('MillepedeCollector_chi2/ndf', 1).obj().getObject('1.1')
pval = Belle2.PyStoreObj('MillepedeCollector_pval', 1).obj().getObject('1.1')

# Skip into interactive environment
# You can draw something in the trees or the profile
# Exit with Ctrl+D
print('++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++')
print(' You are now in interactive environment. You can still access the algorithm')
print(' or the DataStore. Try e.g.:')
print('')
print(' >>> pval.Draw()')
print(' >>> klmtree.Draw("value:layer")')
print(' >>> klm.dump()')
print(' >>> profile.Draw()')
print('')
print(' Look into this script and use TAB or python ? help to play more...')
print(' Exit with [Ctrl] + [D] and then [Enter]')
print('++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++')

import interactive
interactive.embed()
