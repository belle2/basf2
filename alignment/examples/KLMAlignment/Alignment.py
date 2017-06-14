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

gear = register_module('Gearbox')

geom = register_module('Geometry')
geom.param('components', ['PXD', 'SVD'])

main = create_path()
main.add_module(input)
main.add_module(gear)
main.add_module(geom)
process(main)

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
                # if ipar in [1, 2]:
                #     continue

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
            for ipar in range(1, 4):
                eklmid = Belle2.EKLMElementID(endcap, layer, sector)
                label = Belle2.GlobalLabel(eklmid, ipar)
                cmd = str(label.label()) + ' 0. -1.'
                # Uncomment to fix EKLM parameters.
                # algo.steering().command(cmd)
            for plane in range(1, 3):
                for segment in range(1, 6):
                    for ipar in range(1, 3):
                        eklmid = Belle2.EKLMElementID(endcap, layer, sector,
                                                      plane, segment)
                        label = Belle2.GlobalLabel(eklmid, ipar)
                        cmd = str(label.label()) + ' 0. -1.'
                        # Uncomment to fix EKLM parameters.
                        algo.steering().command(cmd)

# ---------- end of parameter fixing ----------------------


# -------------- Run the Millepede Algorithm ----------------

# Execute the algorithm over all collected data (auto-merged)
algo.execute()

# -----------------------------------------------------------
payloads = list(algo.getPayloads())
eklm = None
for payload in payloads:
    if payload.name == 'EKLMAlignment':
        eklm = payload.object.IsA().DynamicCast(Belle2.EKLMAlignment().IsA(), payload.object, False)

# Profile plot for all determined parameters
profile = ROOT.TH1F(
    "profile",
    "correction & errors",
    algo.result().getNoDeterminedParameters(),
    1,
    algo.result().getNoDeterminedParameters())

# Define some branch variables
param = np.zeros(1, dtype=int)
value = np.zeros(1, dtype=np.float32)
correction = np.zeros(1, dtype=np.float32)
error = np.zeros(1, dtype=np.float32)
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
vxdtree.Branch('correction', correction, 'correction/F')
vxdtree.Branch('error', error, 'error/F')
# Tree with CDC data
cdctree = ROOT.TTree('cdc', 'CDC data')
cdctree.Branch('layer', layer, 'layer/I')
cdctree.Branch('param', param, 'param/I')
cdctree.Branch('correction', correction, 'correction/F')
cdctree.Branch('error', error, 'error/F')
# Tree with BKLM data
bklmtree = ROOT.TTree('bklm', 'BKLM data')
bklmtree.Branch('layer', layer, 'layer/I')
bklmtree.Branch('sector', sector, 'sector/I')
bklmtree.Branch('forward', forward, 'forward/I')
bklmtree.Branch('param', param, 'param/I')
bklmtree.Branch('correction', correction, 'correction/F')
bklmtree.Branch('error', error, 'error/F')
# Tree with EKLM sector data.
eklmsectortree = ROOT.TTree('eklm_sector', 'EKLM sector alignment data')
eklmsectortree.Branch('endcap', endcap, 'endcap/I')
eklmsectortree.Branch('layer', layer, 'layer/I')
eklmsectortree.Branch('sector', sector, 'sector/I')
eklmsectortree.Branch('param', param, 'param/I')
eklmsectortree.Branch('value', value, 'value/F')
eklmsectortree.Branch('correction', correction, 'correction/F')
eklmsectortree.Branch('error', error, 'error/F')
# Tree with EKLM segment data.
eklmsegmenttree = ROOT.TTree('eklm_segment', 'EKLM segment alignment data')
eklmsegmenttree.Branch('endcap', endcap, 'endcap/I')
eklmsegmenttree.Branch('layer', layer, 'layer/I')
eklmsegmenttree.Branch('sector', sector, 'sector/I')
eklmsegmenttree.Branch('plane', plane, 'plane/I')
eklmsegmenttree.Branch('segment', segment, 'segment/I')
eklmsegmenttree.Branch('param', param, 'param/I')
eklmsegmenttree.Branch('value', value, 'value/F')
eklmsegmenttree.Branch('correction', correction, 'correction/F')
eklmsegmenttree.Branch('error', error, 'error/F')


# Index of determined param
ibin = 0

for ipar in range(0, algo.result().getNoParameters()):
    label = Belle2.GlobalLabel(algo.result().getParameterLabel(ipar))
    param[0] = label.getParameterId()
    value[0] = 0.0

    if algo.result().isParameterDetermined(ipar):
        correction[0] = algo.result().getParameterCorrection(ipar)
        error[0] = algo.result().getParameterError(ipar)
    else:
        correction[0] = 0.0
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
        if (label.getEklmID().getType() == 2):  # 2 = sector
            endcap[0] = label.getEklmID().getEndcap()
            layer[0] = label.getEklmID().getLayer()
            sector[0] = label.getEklmID().getSector()
            if (eklm is not None):
                alignment = eklm.getSectorAlignment(label.getEklmID().getSectorNumber())
                if (param[0] == 1):
                    value[0] = alignment.getDx()
                elif (param[0] == 2):
                    value[0] = alignment.getDy()
                elif (param[0] == 3):
                    value[0] = alignment.getDalpha()

            eklmsectortree.Fill()
        else:
            endcap[0] = label.getEklmID().getEndcap()
            layer[0] = label.getEklmID().getLayer()
            sector[0] = label.getEklmID().getSector()
            plane[0] = label.getEklmID().getPlane()
            segment[0] = label.getEklmID().getSegment()
            if (eklm is not None):
                alignment = eklm.getSegmentAlignment(label.getEklmID().getSegmentNumber())
                if (param[0] == 1):
                    value[0] = alignment.getDy()
                elif (param[0] == 2):
                    value[0] = alignment.getDalpha()
            eklmsegmenttree.Fill()

    if not algo.result().isParameterDetermined(ipar):
        continue

    ibin = ibin + 1
    profile.SetBinContent(ibin, value[0])
    profile.SetBinError(ibin, error[0])

# Example how to access collected data (but you need exp and run number)
chi2ndf = Belle2.PyStoreObj('MillepedeCollector_chi2/ndf', 1).obj().getObject('1.1')
pval = Belle2.PyStoreObj('MillepedeCollector_pval', 1).obj().getObject('1.1')

alignment_file.cd()
profile.Write()
vxdtree.Write()
cdctree.Write()
bklmtree.Write()
eklmsectortree.Write()
eklmsegmenttree.Write()
alignment_file.Close()

algo.commit()
