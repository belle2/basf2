#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# -----------------------------------------------------------------------------------
#
#                      Example of ECL trigger DQM Module
#
#    usage : %> basf2 TrgEclDQM.py [input sroot file name]
#
# -----------------------------------------------------------------------------------

import basf2 as b2
################
import sys  # get argv
argvs = sys.argv  # get arg
argc = len(argvs)  # of arg
if argc != 2:
    sys.exit("ztsim02.py> # of arg is strange. Exit.")
if argc == 2:
    f_in_root = argvs[1]


b2.set_log_level(b2.LogLevel.INFO)

main = b2.create_path()

# input
input = b2.register_module('RootInput')
input.param('inputFileName', f_in_root)
main.add_module(input)

histo = b2.register_module('HistoManager')
histo.param("histoFileName", "./histo_output.root")

# Unpacker
trgeclUnpacker = b2.register_module("TRGECLUnpacker")
main.add_module(trgeclUnpacker)
main.add_module(histo)

# DQM
trgecldqm = b2.register_module('TRGECLDQM')
main.add_module(trgecldqm)

progress = b2.register_module('Progress')
main.add_module(progress)

b2.process(main)

print(b2.statistics)
