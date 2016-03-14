#!/usr/bin/env python
# -*- coding: utf-8 -*-
#-------------------------------------------------------------------------------------------------------
#           TSim-ecl example code.
#-------------------------------------------------------------------------------------------------------
#       In order to test Tsim-ecl code, you need a root file which has ECLHit table.(after Gsim)
#       ex)
#       commend > basf2 TrgEcl.py [Name of Gsim root file] [Name of output root file]
#-------------------------------------------------------------------------------------------------------
import os
from basf2 import *

################
import sys  # get argv
argvs = sys.argv  # get arg
argc = len(argvs)  # of arg
if argc != 3:
    sys.exit("ztsim02.py> # of arg is strange. Exit.")
if argc == 3:
    f_in_root = argvs[1]
    f_out_root = argvs[2]
# print
# print 'f_in_root  = %s' % f_in_root
# print 'f_out_root = %s\n' % f_out_root

################
#f_in_root1 = f_in_root+ "1.root";
#f_in_root2 = f_in_root+ "2.root";

# suppress messages and warnings during processing:
# level: LogLevel.DEBUG/INFO/WARNING/ERROR/FATALls
set_log_level(LogLevel.ERROR)
# set_log_level(LogLevel.INFO)
# set_log_level(LogLevel.DEBUG)

# one event
#eventinfosetter.param({'evtNumList': [1000], 'runList': [1]})


gearbox = register_module('Gearbox')

# input
rootinput1 = register_module('RootInput')
rootinput1.param('inputFileName', f_in_root)

# TRGECL
trgeclfam = register_module("TRGECLFAM")
trgecl = register_module("TRGECL")
trgeclMC = register_module("MCMatcherTRGECL")

# output
rootoutput = register_module('RootOutput')
rootoutput.param('outputFileName', f_out_root)

# import random
progress = register_module('Progress')


# Set parameters

# Create paths
main = create_path()

main.add_module(rootinput1)
# main.add_module(rootinput2)

main.add_module(progress)
main.add_module(gearbox)

main.add_module(trgeclfam)
main.add_module(trgecl)
main.add_module(trgeclMC)

main.add_module(rootoutput)


# main
process(main)
###
###
###
# print statistics
# ===<END>
