#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# -----------------------------------------------------------------------------------
#
#                      Example of GDL trigger DQM Module
#
#    usage : %> basf2 trggdlDQM.py [input sroot file name]
#
# -----------------------------------------------------------------------------------

import basf2 as b2
################
import sys  # get argv
import re
import os.path
argvs = sys.argv  # get arg
argc = len(argvs)  # of arg

b2.set_log_level(b2.LogLevel.DEBUG)

b2.use_central_database("TRGGDL_201811")

main = b2.create_path()

if argc == 2 and argvs[1][-6:] == ".sroot":
    f_in_root = argvs[1]
    input = b2.register_module('SeqRootInput')
    matchobj = re.search("([^\\/]+)\\.sroot", f_in_root)
    basename = re.sub('\\.sroot$', '', matchobj.group())
    input.param('inputFileName', f_in_root)
elif argc == 2 and argvs[1][-5:] == ".root":
    f_in_root = argvs[1]
    input = b2.register_module('RootInput')
    matchobj = re.search("([^\\/]+)\\.root", f_in_root)
    basename = re.sub('\\.root$', '', matchobj.group())
    input.param('inputFileName', f_in_root)
elif argc == 1:
    input = b2.register_module('RootInput')
    input.param('inputFileName', '/home/belle/nkzw/e3.4S/r034*/all/raw/sub00/raw.physics.hlt_hadron.0003.*.root')
    basename = "e3.4S.r034"
else:
    sys.exit("trggdlDQM.py> # of arg is strange. Exit.")

main.add_module(input)
histo = b2.register_module('HistoManager')
histo.param("histoFileName", "dqm.%s.root" % basename)

# Unpacker
trggdlUnpacker = b2.register_module("TRGGDLUnpacker")
main.add_module(trggdlUnpacker)
main.add_module(histo)

# DQM
# trggdldqm = register_module('TRGGDLDQM', logLevel=LogLevel.DEBUG, debugLevel=20)
trggdldqm = b2.register_module('TRGGDLDQM')
# event by event bit-vs-clock TH2I hist for itd, ftdl, psnm in ROOT file.
trggdldqm.param('eventByEventTimingHistRecord', False)
# bit name on BinLabel for hGDL_itd,ftd,psn.
trggdldqm.param('bitNameOnBinLabel', True)
# generate postscript file that includes rising and falling edge distribution
trggdldqm.param('generatePostscript', False)
# postscript file name
psname = "dqm.%s.ps" % basename
trggdldqm.param('postScriptName', psname)

# dump vcd file
dumpVcdFileTrue = False
trggdldqm.param('dumpVcdFile', dumpVcdFileTrue)
if dumpVcdFileTrue:
    if not os.path.isdir('vcd'):
        os.mkdir('vcd')
    trggdldqm.param('bitConditionToDumpVcd', 'HIE ECL_BHA')
    # '+' means logical OR. '!' is logical NOT.
    # Bit names is delimited by space. No space after '!'.
    # Parenthesis, A (B+!C) not allowed. Must be expanded to 'A B+A !C'.
    # If one of characters is capital, the bit is regarded as psnm bit,
    # otherwise ftdl bit: 'hie' is ftdl hie bit. 'hIE' is psnm hie bit.
    trggdldqm.param('vcdEventStart', 0)
    trggdldqm.param('vcdNumberOfEvents', 10)

main.add_module(trggdldqm)

progress = b2.register_module('Progress')
main.add_module(progress)

b2.process(main)

print(b2.statistics)
