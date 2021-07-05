#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# -----------------------------------------------------------------------------------
#
#                      Example of GDL trigger DQM Module
#
#    usage : %> basf2 trgcdctsfDQM.py [input sroot file name]
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
argvs = sys.argv  # get arg
argc = len(argvs)  # of arg

b2.set_log_level(b2.LogLevel.DEBUG)

main = b2.create_path()

if argc == 2 and argvs[1][-6:] == ".sroot":
    f_in_root = argvs[1]
    input = b2.register_module('SeqRootInput')
    matchobj = re.search(r"([^\/]+)\.sroot", f_in_root)
    basename = re.sub(r'\.sroot$', '', matchobj.group())
    input.param('inputFileName', f_in_root)
elif argc == 2 and argvs[1][-5:] == ".root":
    f_in_root = argvs[1]
    input = b2.register_module('RootInput')
    matchobj = re.search(r"([^\/]+)\.root", f_in_root)
    basename = re.sub(r'\.root$', '', matchobj.group())
    input.param('inputFileName', f_in_root)
elif argc == 1:
    input = b2.register_module('RootInput')
    input.param('inputFileName', '/home/belle/nkzw/e3.4S/r034*/all/raw/sub00/raw.physics.hlt_hadron.0003.*.root')
    basename = "e3.4S.r034"
else:
    sys.exit("trgcdctsfDQM.py> # of arg is strange. Exit.")

main.add_module(input)
histo = b2.register_module('HistoManager')
histo.param("histoFileName", "dqm.%s.superlayer4.root" % basename)

# Unpacker
trgcdctsfUnpacker = b2.register_module("TRGCDCTSFUnpacker")
trgcdctsfUnpacker.param('TSFMOD', 4)
main.add_module(trgcdctsfUnpacker)
main.add_module(histo)

# DQM
trgcdctsfdqm = b2.register_module('TRGCDCTSFDQM')
trgcdctsfdqm.param('generatePostscript', True)
trgcdctsfdqm.param('TSFMOD', 4)
# postscript file name
psname = "dqm.%s.ps" % basename
trgcdctsfdqm.param('postScriptName', psname)

main.add_module(trgcdctsfdqm)

progress = b2.register_module('Progress')
main.add_module(progress)

b2.process(main)

print(b2.statistics)
