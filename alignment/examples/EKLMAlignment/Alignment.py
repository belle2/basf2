#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from basf2 import *
import ROOT
from ROOT import Belle2
import numpy as np
from alignment import MillepedeCalibration, calibrate
from caf.framework import Calibration

set_log_level(LogLevel.DEBUG)

# Create the algorithm
millepede = MillepedeCalibration(['EKLMAlignment'])

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
                # millepede.fixGlobalParam(Belle2.EKLMAlignment.getGlobalUniqueID(),
                #                         eklmid.getGlobalNumber(),
                #                         ipar)
            for plane in range(1, 3):
                for segment in range(1, 6):
                    for ipar in range(1, 3):
                        eklmid = Belle2.EKLMElementID(endcap, layer, sector, plane, segment)
                        millepede.fixGlobalParam(Belle2.EKLMAlignment.getGlobalUniqueID(),
                                                 eklmid.getGlobalNumber(),
                                                 ipar)

# ---------- end of parameter fixing ----------------------


# -------------- Run the Millepede Algorithm ----------------

# Execute the algorithm over all collected data (auto-merged)
calibration = millepede.create('eklm_alignment', sys.argv[1])
calibrate(calibration, sys.argv[1])

# -----------------------------------------------------------
payloads = list(millepede.algo.getPayloads())
eklm = None
for payload in payloads:
    if payload.name == 'EKLMAlignment':
        eklm = payload.object.IsA().DynamicCast(Belle2.EKLMAlignment().IsA(), payload.object, False)

# Profile plot for all determined parameters
profile = ROOT.TH1F(
    "profile",
    "correction & errors",
    millepede.algo.result().getNoDeterminedParameters(),
    1,
    millepede.algo.result().getNoDeterminedParameters())

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

for ipar in range(0, millepede.algo.result().getNoParameters()):
    label = Belle2.GlobalLabel(millepede.algo.result().getParameterLabel(ipar))
    param[0] = label.getParameterId()
    value[0] = 0.0

    if millepede.algo.result().isParameterDetermined(ipar):
        correction[0] = millepede.algo.result().getParameterCorrection(ipar)
        error[0] = millepede.algo.result().getParameterError(ipar)
    else:
        correction[0] = 0.0
        error[0] = -1.0

    if (label.getUniqueId() == 40):  # EKLMAlignment
        eklmid = Belle2.EKLMElementID(label.getElementId())
        if (eklmid.getType() == 2):  # 2 = sector
            endcap[0] = eklmid.getEndcap()
            layer[0] = eklmid.getLayer()
            sector[0] = eklmid.getSector()
            if (eklm is not None):
                alignment = eklm.getSectorAlignment(eklmid.getSectorNumber())
                if (param[0] == 1):
                    value[0] = alignment.getDx()
                elif (param[0] == 2):
                    value[0] = alignment.getDy()
                elif (param[0] == 6):
                    value[0] = alignment.getDalpha()

            eklmsectortree.Fill()
        else:
            endcap[0] = eklmid.getEndcap()
            layer[0] = eklmid.getLayer()
            sector[0] = eklmid.getSector()
            plane[0] = eklmid.getPlane()
            segment[0] = eklmid.getSegment()
            if (eklm is not None):
                alignment = eklm.getSegmentAlignment(eklmid.getSegmentNumber())
                if (param[0] == 1):
                    value[0] = alignment.getDy()
                elif (param[0] == 2):
                    value[0] = alignment.getDalpha()
            eklmsegmenttree.Fill()

    if not millepede.algo.result().isParameterDetermined(ipar):
        continue

    ibin = ibin + 1
    profile.SetBinContent(ibin, value[0])
    profile.SetBinError(ibin, error[0])

alignment_file.cd()
profile.Write()
eklmsectortree.Write()
eklmsegmenttree.Write()
alignment_file.Close()
