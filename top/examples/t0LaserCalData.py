#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# -------------------------------------------------------------------------------------------
# example of generate TOP channel t0 const from TOP digit time with TBC
# input: "t0mc.root" from TOPChannelT0MC
# output: "t0datafit_slot<#>.root" fit result
#         "t0const_slot<#>.root" t0 const in root file
# Usage: basf2 t0LaserCalData.py -i <TOPDigits.root> <slot#>
#        TOPDigits.root is the output file of unpackToTOPDigitsWithTBC.py
# -------------------------------------------------------------------------------------------

import basf2 as b2
import sys
args = sys.argv
barid = int(args[1])

# Create path
main = b2.create_path()

# Input
roinput = b2.register_module('RootInput')
main.add_module(roinput)

lasercalib = b2.register_module('TOPLaserCalibrator')
lasercalib.param('dataFitOutput', "t0datafit_slot" + str(args[1]) + ".root")
lasercalib.param('mcInput', "t0mc.root")
lasercalib.param('channelT0constant', "t0const_slot" + str(args[1]) + ".root")
lasercalib.param('barID', barid)
lasercalib.param('fitMethod', 'cb')  # gaus: single gaussian; cb: single Crystal Ball(for MC test); cb2: double Crystal Ball
lasercalib.param('fitRange', [480, 66, 82])  # fit range[nbins, xmin, xmax]
lasercalib.param('refCh', 0)  # reference channel of t0 constant
main.add_module(lasercalib)


# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
