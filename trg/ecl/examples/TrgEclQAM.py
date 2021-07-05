#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# -----------------------------------------------------------------------------------
#
#                      Example of ECL trigger DQM Module
#
#    usage : %> basf2 TrgEclDQM.py [input sroot file name]
#
# -----------------------------------------------------------------------------------

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
################
import sys  # get argv
argvs = sys.argv  # get arg
argc = len(argvs)  # of arg
if argc != 3:
    sys.exit("ztsim02.py> # of arg is strange. Exit.")
if argc == 3:
    f_in_root = argvs[1]
    f_out_root = argvs[2]


b2.set_log_level(b2.LogLevel.INFO)

main = b2.create_path()

# input
input = b2.register_module('RootInput')
input.param('inputFileName', f_in_root)
main.add_module(input)


# Unpacker
trgeclUnpacker = b2.register_module("TRGECLUnpacker")
trggdlUnpacker = b2.register_module("TRGGDLUnpacker")
trggdlsummary = b2.register_module("TRGGDLSummary")

main.add_module(trgeclUnpacker)
main.add_module(trggdlUnpacker)
main.add_module(trggdlsummary)


# QAM
trgeclqam = b2.register_module('TRGECLQAM')
trgeclqam.param('outputFileName', f_out_root)
main.add_module(trgeclqam)

progress = b2.register_module('Progress')
main.add_module(progress)

b2.process(main)

print(b2.statistics)
