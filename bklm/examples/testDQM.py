#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#################################################################
#                                                               #
#    script to create DQM histograms from an input file that    #
#    contains BKLMDigits.                                       #
#                                                               #
#    written by Vipin Gaur, VT                                  #
#    vipinhep@vt.edu                                            #
#                                                               #
#################################################################

"""
<header>
    <output>BKLMDQM.root</output>
    <contact>vipinhep@vt.edu</contact>
    <description>Create BKLM DQM histograms from BKLMDigits.</description>
</header>
"""

from basf2 import *

# set_log_level(LogLevel.DEBUG)

# Register modules
input = register_module("RootInput")
histo = register_module("HistoManager")
mymodule = register_module("BKLMDQM")

# Create paths
main = create_path()

# Add modules to path
main.add_module(input)
main.add_module(histo)
main.add_module(mymodule)

# Set parameters
input.param("inputFileName", "muon-BKLMDigits.root")
histo.param("histoFileName", "BKLMDQM.root")

# Process events
process(main)
