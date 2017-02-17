#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from basf2 import *
import ROOT
from ROOT import Belle2
import numpy as np

main = create_path()

input = register_module('RootInput')
input.param('inputFileName', sys.argv[1])
input.initialize()

gear = register_module('Gearbox')
gear.initialize()
geom = register_module('Geometry')
geom.param('components', ['PXD', 'SVD'])
geom.initialize()

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
                if ipar in [1, 2]:
                    continue

                bklmid = Belle2.BKLMElementID()
                bklmid.setIsForward(forward)
                bklmid.setSectorNumber(sector)
                bklmid.setLayerNumber(layer)
                label = Belle2.GlobalLabel(bklmid, ipar)
                cmd = str(label.label()) + ' 0. -1.'
                algo.steering().command(cmd)

# Fix EKLM parameters
for endcap in range(1, 3):
    if (endcap == 1):
        maxlayer = 12
    else:
        maxlayer = 14
    for layer in range(1, maxlayer + 1):
        for sector in range(1, 5):
            for plane in range(1, 3):
                for segment in range(1, 6):
                    for ipar in range(1, 3):
                        eklmid = Belle2.EKLMSegmentID(endcap, layer, sector,
                                                      plane, segment)
                        label = Belle2.GlobalLabel(eklmid, ipar)
                        cmd = str(label.label()) + ' 0. -1.'
                        # Uncomment to fix EKLM parameters.
                        # algo.steering().command(cmd)

# ---------- end of parameter fixing ----------------------


# -------------- Run the Millepede Algorithm ----------------

# Execute the algorithm over all collected data (auto-merged)
algo.execute()

# -----------------------------------------------------------

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
endcap = np.zeros(1, dtype=int)
plane = np.zeros(1, dtype=int)
segment = np.zeros(1, dtype=int)

alignment_file = ROOT.TFile('alignment.root', 'recreate')
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
# Tree with EKLM data.
eklmtree = ROOT.TTree('eklm', 'EKLM data')
eklmtree.Branch('endcap', endcap, 'endcap/I')
eklmtree.Branch('layer', layer, 'layer/I')
eklmtree.Branch('sector', sector, 'sector/I')
eklmtree.Branch('plane', plane, 'plane/I')
eklmtree.Branch('segment', segment, 'segment/I')
eklmtree.Branch('param', param, 'param/I')
eklmtree.Branch('value', value, 'value/D')
eklmtree.Branch('error', error, 'error/D')

# Index of determined param
ibin = 0

for ipar in range(0, algo.result().getNoParameters()):
    label = Belle2.GlobalLabel(algo.result().getParameterLabel(ipar))
    param[0] = label.getParameterId()

    if algo.result().isParameterDetermined(ipar):
        value[0] = algo.result().getParameterCorrection(ipar)
        error[0] = algo.result().getParameterError(ipar)
    else:
        value[0] = 0.0
        error[0] = -1.0

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

    if (label.isEKLM()):
        endcap[0] = label.getEklmID().getEndcap()
        layer[0] = label.getEklmID().getLayer()
        sector[0] = label.getEklmID().getSector()
        plane[0] = label.getEklmID().getPlane()
        segment[0] = label.getEklmID().getSegment()
        eklmtree.Fill()

    if not algo.result().isParameterDetermined(ipar):
        continue

    ibin = ibin + 1
    profile.SetBinContent(ibin, value[0])
    profile.SetBinError(ibin, error[0])

# Example how to access collected data (but you need exp and run number)
chi2ndf = Belle2.PyStoreObj('MillepedeCollector_chi2/ndf', 1).obj().getObject('1.1')
pval = Belle2.PyStoreObj('MillepedeCollector_pval', 1).obj().getObject('1.1')

alignment_file.cd()
vxdtree.Write()
cdctree.Write()
bklmtree.Write()
eklmtree.Write()
alignment_file.Close()

algo.commit()
