#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2

b2.set_log_level(b2.LogLevel.INFO)

# input
input = b2.register_module('SeqRootInput')

# dump
# dump = register_module('RootOutput')

# progress monitor
progress = b2.register_module('Progress')

# converts RawCOPPER->RawARICH
convert = b2.register_module('Convert2RawDet')

# converts RawARICH->ARICHRawDigits
unpack = b2.register_module('ARICHRawunpacker')

# Create main path
main = b2.create_path()

# Add modules to main path
main.add_module(input)
main.add_module(convert)
main.add_module(unpack)
# main.add_module(dump)
main.add_module(progress)

# Process all events
b2.process(main)

print(b2.statistics)
