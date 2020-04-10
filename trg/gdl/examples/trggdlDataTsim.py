#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# -----------------------------------------------------------------------------------
#
#                      Example of GDL trigger Summary Module
#
#    usage : %> basf2 trggdlSummary.py [input sroot file name]
#
# -----------------------------------------------------------------------------------

import basf2
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

# 0
# use_central_database("online")
# use_local_database("./localdb/database.txt")
# use_central_database("staging_online")

# use_central_database("master_2019-09-26")
# use_central_database("data_reprocessing_prompt_bucket6")
# use_central_database("TRGGDL_201811")
# use_central_database("online")
# use_central_database("staging_online")

# 1
basf2.conditions.disable_globaltag_replay()
basf2.conditions.prepend_testing_payloads('./localdb/database.txt')

# 2
# basf2.conditions.disable_globaltag_replay()
# basf2.conditions.globaltags = ['data_reprocessing_prompt_rel4_patchb', 'online']
# basf2.conditions.prepend_testing_payloads('./localdb/database.txt')

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


# Unpacker
trggdlUnpacker = register_module("TRGGDLUnpacker")
# trggdlUnpacker.param('print_dbmap', True)
main.add_module(trggdlUnpacker)

#
trggdlsummary = register_module('TRGGDLSummary')
main.add_module(trggdlsummary)

datatsim = register_module('TRGGDL')
datatsim.param('SimulationMode', 3)
datatsim.param('Belle2Phase', "Belle2Phase")
# datatsim.param('algFromDB', False)
# datatsim.param('algFilePath', "ftd_0023.alg")
main.add_module(datatsim)

histo = register_module('HistoManager')
histo.param("histoFileName", "trgsum/dsim.%s.root" % basename)
main.add_module(histo)
if not os.path.isdir('trgsum'):
    os.mkdir('trgsum')

progress = register_module('Progress')
main.add_module(progress)

# main.add_module(output, branchNames=["TRGSummary"])

process(main)
# process(main, max_event=200)

print(statistics)