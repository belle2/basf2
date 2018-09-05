#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# -----------------------------------------------------------------------------------
#
#                      Example of GDL trigger DQM Module
#
#    usage : %> basf2 trggdlDQM.py [input sroot file name]
#
# -----------------------------------------------------------------------------------

from basf2 import *
################
import sys  # get argv
import re
import os.path
argvs = sys.argv  # get arg
argc = len(argvs)  # of arg
if argc != 2:
    sys.exit("trggdlDQM.py> # of arg is strange. Exit.")
if argc == 2:
    f_in_root = argvs[1]


set_log_level(LogLevel.DEBUG)

main = create_path()

if f_in_root[-6:] == ".sroot":
    rootfiletype = "sroot"
    input = register_module('SeqRootInput')
    matchobj = re.search("([^\/]+)\.sroot", f_in_root)
    basename = re.sub('\.sroot$', '', matchobj.group())
if f_in_root[-5:] == ".root":
    rootfiletype = "root"
    input = register_module('RootInput')
    matchobj = re.search("([^\/]+)\.root", f_in_root)
    basename = re.sub('\.root$', '', matchobj.group())

input.param('inputFileName', f_in_root)
main.add_module(input)

histo = register_module('HistoManager')
histo.param("histoFileName", "dqm.%s.root" % basename)

# Unpacker
trggdlUnpacker = register_module("TRGGDLUnpacker")
main.add_module(trggdlUnpacker)
main.add_module(histo)

# DQM
trggdldqm = register_module('TRGGDLDQM')
trggdldqm.param('eventByEventTimingHistRecord', False)
# event by event bit-vs-clock TH2I hist for itd, ftdl, psnm in ROOT file.

# dump vcd file
dumpVcdFileTrue = False
trggdldqm.param('dumpVcdFile', dumpVcdFileTrue)
if dumpVcdFileTrue:
    if not os.path.isdir('vcd'):
        os.mkdir('vcd')
    trggdldqm.param('bitConditionToDumpVcd', 'fff + !fffo')
    # '+' means logical OR. '!' is logical NOT.
    # Bit names is delimited by space. No space after '!'.
    # Parenthesis, A (B+!C) not allowed. Must be expanded to 'A B+A !C'.
    # If one of characters is capital, the bit is regarded as psnm bit,
    # otherwise ftdl bit: 'hie' is ftdl hie bit. 'hIE' is psnm hie bit.
    trggdldqm.param('vcdEventStart', 10)
    trggdldqm.param('vcdNumberOfEvents', 5)

main.add_module(trggdldqm)

progress = register_module('Progress')
main.add_module(progress)

process(main)

print(statistics)
