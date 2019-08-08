#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# -----------------------------------------------------------------------------------
#
#                      Example of GDL trigger Summary Module
#
#    usage : %> basf2 trggdlSummary.py [input sroot file name]
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
    sys.exit("trggdlSummary.py> # of arg is strange. Exit.")
if argc == 2:
    f_in_root = argvs[1]


set_log_level(LogLevel.INFO)

# use_central_database("online")
# use_central_database("staging_online")
# use_central_database("TRGGDL_201811")

main = create_path()

# input
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


# output
output = register_module('RootOutput')
output.param("outputFileName", "trgsum/trgsum.%s.root" % basename)
if not os.path.isdir('trgsum'):
    os.mkdir('trgsum')

# Unpacker
trggdlUnpacker = register_module("TRGGDLUnpacker")
main.add_module(trggdlUnpacker)

#
trggdlsummary = register_module('TRGGDLSummary')
main.add_module(trggdlsummary)

datatsim = register_module('TRGGDL')
datatsim.param('SimulationMode', 3)
datatsim.param('Belle2Phase', "Belle2Phase")
datatsim.param('algFromDB', False)
main.add_module(datatsim)

progress = register_module('Progress')
main.add_module(progress)

main.add_module(output, branchNames=["TRGSummary"])

process(main)

print(statistics)
