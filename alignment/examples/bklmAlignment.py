#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from basf2 import *
import ROOT
from ROOT import Belle2
from alignment import MillepedeCalibration, calibrate
import numpy as np

use_local_database('localdb/database.txt')

components = ROOT.vector('string')()
components.clear()
components.push_back('BKLMAlignment')

algo = Belle2.MillepedeAlgorithm()
algo.setInputFileNames([sys.argv[1]])
algo.setComponents(components)


# Now fix some parameters
algo.steering().command('Parameters')

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

# ---------- end of parameter fixing ----------------------


# -------------- Run the Millepede Algorithm ----------------

# Execute the algorithm over all collected data (auto-merged)
algo.execute()

# -----------------------------------------------------------


# Get the payloads into handy variables
payloads = list(algo.getPayloads())
bklm = None
for payload in payloads:
    if payload.name == 'BKLMAlignment':
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
