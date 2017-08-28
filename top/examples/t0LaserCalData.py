#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

gb2_setuprel = 'build-2017-06-14'

# -------------------------------------------------------------------------------------------
# example of generate TOP channel t0 const from TOP digit time with TBC
# input: "t0mc.root" from TOPChannelT0MC
# output: "t0datafit.root" fit result
#         "t0const.root" t0 const in root file
# Usage: basf2 t0LaserCalData.py -i <TOPDigits_withTBC.root>
#   e.g. basf2 t0LaserCalData.py -i ~yuanwl/public/data/run004881_TBC4855-4858_slot01.root
# -------------------------------------------------------------------------------------------

from sys import argv
# Create path
main = create_path()

# Input
roinput = register_module('RootInput')
main.add_module(roinput)

lasercalib = register_module('TOPLaserCalibrator')
lasercalib.param('dataFitOutput', "t0datafit.root")
lasercalib.param('mcInput', "t0mc.root")
lasercalib.param('channelT0constant', "t0const.root")
lasercalib.param('barID', 1)
# lasercalib.param('fitChannel', 142)
lasercalib.param('fitMethod', 'cb')  # gauss: single gaussian; cb: single Crystal Ball(for MC test); cb2: double Crystal Ball
lasercalib.param('fitRange', [100, -14, -10])  # fit range[nbins, xmin, xmax]
main.add_module(lasercalib)


# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
