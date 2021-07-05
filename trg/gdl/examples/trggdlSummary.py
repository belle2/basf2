#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# -----------------------------------------------------------------------------------
#
#                      Example of GDL trigger Summary Module
#
#    usage : %> basf2 trggdlSummary.py [input sroot file name]
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
import re
import os.path
argvs = sys.argv  # get arg
argc = len(argvs)  # of arg
if argc != 2:
    sys.exit("trggdlSummary.py> # of arg is strange. Exit.")
if argc == 2:
    f_in_root = argvs[1]


b2.set_log_level(b2.LogLevel.INFO)

main = b2.create_path()

# input
if f_in_root[-6:] == ".sroot":
    rootfiletype = "sroot"
    input = b2.register_module('SeqRootInput')
    matchobj = re.search("([^\\/]+)\\.sroot", f_in_root)
    basename = re.sub('\\.sroot$', '', matchobj.group())
if f_in_root[-5:] == ".root":
    rootfiletype = "root"
    input = b2.register_module('RootInput')
    matchobj = re.search("([^\\/]+)\\.root", f_in_root)
    basename = re.sub('\\.root$', '', matchobj.group())

input.param('inputFileName', f_in_root)
main.add_module(input)


# output
output = b2.register_module('RootOutput')
output.param("outputFileName", "trgsum/trgsum.%s.root" % basename)
if not os.path.isdir('trgsum'):
    os.mkdir('trgsum')

# Unpacker
trggdlUnpacker = b2.register_module("TRGGDLUnpacker")
main.add_module(trggdlUnpacker)

#
trggdlsummary = b2.register_module('TRGGDLSummary')
main.add_module(trggdlsummary)

progress = b2.register_module('Progress')
main.add_module(progress)

main.add_module(output, branchNames=["TRGSummary"])

b2.process(main)

print(b2.statistics)
