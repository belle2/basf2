#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################################
#
# This steering file expects a simulated data sample at innput and
# perfoms the standard reconstruction. The MillepedeCollector
# is used to re-fit tracks with General Broken Lines
#
# Finally the display is run to demonstrate its capability to visualize tracks
# fitted with GBL (done as byproduct of MillepedeCollector module)
#
##############################################################################
##

import basf2 as b2
import reconstruction

import ROOT
ROOT.gROOT.SetBatch(0)

main = b2.create_path()

main.add_module('RootInput')
main.add_module('HistoManager', histoFileName='CollectorOutput.root')
main.add_module('Gearbox')
main.add_module('Geometry')

reconstruction.add_reconstruction(main, pruneTracks=False, add_muid_hits=True)
main.add_module('DAFRecoFitter')

main.add_module(
    'MillepedeCollector',
    components=[],
    tracks=['RecoTracks'])

main.add_module('Display', showRecoTracks=True)
# main.add_module('RootOutput')
b2.process(main)
print(b2.statistics)
