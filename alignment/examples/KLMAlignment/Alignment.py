#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# KLM alignment: alignment using the collected data.

import sys
import basf2
import ROOT
from ROOT import Belle2
from ROOT.Belle2 import KLMChannelIndex, KLMElementNumbers
import numpy as np
from alignment import MillepedeCalibration

basf2.set_log_level(basf2.LogLevel.INFO)

basf2.conditions.append_testing_payloads('localdb/database.txt')

# Create the algorithm.
millepede = MillepedeCalibration(['BKLMAlignment', 'EKLMAlignment', 'EKLMSegmentAlignment'])

# Fix module parameters.
index = KLMChannelIndex(KLMChannelIndex.c_IndexLevelLayer)
index2 = KLMChannelIndex(KLMChannelIndex.c_IndexLevelLayer)
while (index != index2.end()):
    module = index.getKLMModuleNumber()
    if (index.getSubdetector() == KLMElementNumbers.c_BKLM):
        for ipar in [1, 2, 3, 4, 5, 6]:
            # Free parameters are idU, dV, dGamma.
            if ipar in [1, 2, 6]:
                continue
            millepede.fixGlobalParam(Belle2.BKLMAlignment.getGlobalUniqueID(),
                                     module, ipar)
    else:
        for ipar in [1, 2, 6]:
            # No parameters are fixed; if necessary, uncomment the following:
            # millepede.fixGlobalParam(Belle2.EKLMAlignment.getGlobalUniqueID(),
            #                          module, ipar)
            continue
    index.increment()

# Fix EKLM segment parameters.
index.setIndexLevel(KLMChannelIndex.c_IndexLevelStrip)
index2.setIndexLevel(KLMChannelIndex.c_IndexLevelStrip)
index = index2.beginEKLM()
index.useEKLMSegments()
while (index != index2.endEKLM()):
    segment = index.getEKLMSegmentNumber()
    for ipar in [2, 6]:
        millepede.fixGlobalParam(
            Belle2.EKLMSegmentAlignment.getGlobalUniqueID(),
            segment, ipar)
    index.increment()

# Execute the algorithm over all collected data (auto-merged).
input_files = sys.argv[1:]
calibration = millepede.create('klm_alignment', input_files)
millepede.algo.setInputFileNames(input_files)
millepede.algo.ignoreUndeterminedParams(True)
millepede.algo.invertSign()
millepede.algo.execute()
millepede.algo.commit()

# Get payloads.
payloads = list(millepede.algo.getPayloads())
bklm_alignment = None
eklm_alignment = None
eklm_segment_alignment = None
for payload in payloads:
    if payload.name == 'BKLMAlignment':
        bklm_alignment = payload.object.IsA().DynamicCast(Belle2.BKLMAlignment().IsA(), payload.object, False)
    elif payload.name == 'EKLMAlignment':
        eklm_alignment = payload.object.IsA().DynamicCast(Belle2.EKLMAlignment().IsA(), payload.object, False)
    elif payload.name == 'EKLMAlignment':
        eklm_segment_alignment = payload.object.IsA().DynamicCast(Belle2.EKLMSegmentAlignment().IsA(), payload.object, False)

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
section = np.zeros(1, dtype=int)
sector = np.zeros(1, dtype=int)
layer = np.zeros(1, dtype=int)
sensor = np.zeros(1, dtype=int)
plane = np.zeros(1, dtype=int)
segment = np.zeros(1, dtype=int)

alignment_file = ROOT.TFile('alignment.root', 'recreate')
# Tree with BKLM module data.
bklm_module_tree = ROOT.TTree('bklm_module', 'BKLM module alignment data')
bklm_module_tree.Branch('section', section, 'section/I')
bklm_module_tree.Branch('sector', sector, 'sector/I')
bklm_module_tree.Branch('layer', layer, 'layer/I')
bklm_module_tree.Branch('param', param, 'param/I')
bklm_module_tree.Branch('value', value, 'value/F')
bklm_module_tree.Branch('correction', correction, 'correction/F')
bklm_module_tree.Branch('error', error, 'error/F')
# Tree with EKLM module data.
eklm_module_tree = ROOT.TTree('eklm_module', 'EKLM module alignment data')
eklm_module_tree.Branch('section', section, 'section/I')
eklm_module_tree.Branch('sector', sector, 'sector/I')
eklm_module_tree.Branch('layer', layer, 'layer/I')
eklm_module_tree.Branch('param', param, 'param/I')
eklm_module_tree.Branch('value', value, 'value/F')
eklm_module_tree.Branch('correction', correction, 'correction/F')
eklm_module_tree.Branch('error', error, 'error/F')
# Tree with EKLM segment data.
eklm_segment_tree = ROOT.TTree('eklm_segment', 'EKLM segment alignment data')
eklm_segment_tree.Branch('section', section, 'section/I')
eklm_segment_tree.Branch('sector', sector, 'sector/I')
eklm_segment_tree.Branch('layer', layer, 'layer/I')
eklm_segment_tree.Branch('plane', plane, 'plane/I')
eklm_segment_tree.Branch('segment', segment, 'segment/I')
eklm_segment_tree.Branch('param', param, 'param/I')
eklm_segment_tree.Branch('value', value, 'value/F')
eklm_segment_tree.Branch('correction', correction, 'correction/F')
eklm_segment_tree.Branch('error', error, 'error/F')

# Index of determined parameter.
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

    # Module alignment.
    if (label.getUniqueId() == Belle2.BKLMAlignment.getGlobalUniqueID() or
            label.getUniqueId() == Belle2.EKLMAlignment.getGlobalUniqueID()):
        module = label.getElementId()
        index.setKLMModule(module)
        section[0] = index.getSection()
        sector[0] = index.getSector()
        layer[0] = index.getLayer()
        if (index.getSubdetector() == KLMElementNumbers.c_BKLM):
            if (bklm_alignment is not None):
                value[0] = bklm_alignment.getGlobalParam(module, int(param[0]))
            bklm_module_tree.Fill()
        else:
            if (eklm_alignment is not None):
                value[0] = eklm_alignment.getGlobalParam(module, int(param[0]))
            eklm_module_tree.Fill()

    # EKLM segments alignment.
    elif (label.getUniqueId() ==
          Belle2.EKLMSegmentAlignment.getGlobalUniqueID()):
        segment_global = label.getElementId()
        index.setEKLMSegment(segment_global)
        section[0] = index.getSection()
        sector[0] = index.getSector()
        layer[0] = index.getLayer()
        plane[0] = index.getPlane()
        segment[0] = index.getStrip()
        if (eklm_segment_alignment is not None):
            value[0] = eklm_segment_alignment.getGlobalParam(segment, int(param[0]))
        eklm_segment_tree.Fill()

    if not millepede.algo.result().isParameterDetermined(ipar):
        continue

    ibin = ibin + 1
    profile.SetBinContent(ibin, value[0])
    profile.SetBinError(ibin, error[0])

alignment_file.cd()
profile.Write()
bklm_module_tree.Write()
eklm_module_tree.Write()
eklm_segment_tree.Write()
alignment_file.Close()
