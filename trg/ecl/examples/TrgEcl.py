#!/usr/bin/env python
# -*- coding: utf-8 -*-
# -------------------------------------------------------------------------------------------------------
#            TSim-ecl example code.
# -------------------------------------------------------------------------------------------------------
#        In order to test Tsim-ecl code, you need a root file which has ECLHit table.(after Gsim)
#        ex)
#        commend > basf2 TrgEcl.py [Name of Gsim root file] [Name of output root file]
# -------------------------------------------------------------------------------------------------------
import basf2 as b2
from L1trigger import add_trigger_simulation
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

b2.set_log_level(b2.LogLevel.ERROR)
# set_log_level(LogLevel.INFO)
# set_log_level(LogLevel.DEBUG)
# use_local_database('./trg_ecl/database.txt')

gearbox = b2.register_module('Gearbox')

# input
rootinput1 = b2.register_module('RootInput')
rootinput1.param('inputFileName', f_in_root)


# output
rootoutput = b2.register_module('RootOutput')
rootoutput.param('outputFileName', f_out_root)

# import random
progress = b2.register_module('Progress')


# Create paths
main = b2.create_path()

main.add_module(rootinput1)

main.add_module(progress)
main.add_module(gearbox)

add_trigger_simulation(main, component=["ECL"])


main.add_module(rootoutput, branchNames=["TRGECLTrgs", "TRGECLHits", "TRGECLClusters"])


# main
b2.process(main)
###
###
###
print(b2.statistics)
# ===<END>
