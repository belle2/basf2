#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# This is example script to run extract ARICH background relevant data
# from prepared background samples. In runs ARICHBackground module and
# produces root file with TTree, needed to produce arich background plots.
#
# Please check and adjust input file list and method, as this example is
# specific for background files produced by Nakayama-san.
#
# Author: Luka Santelj
#

import os
import sys
from basf2 import *
logging.log_level = LogLevel.WARNING

print('')
print('Use the script as: basf2 ARICHBkg.py arguments')
print('Arguments are in the following order: type (RBB,Touschek_HER,...), path to files , '
      'job number , number of input files , background tag')
print('')
print('example: basf2 ARICHBkg.py RBB /gpfs/home/belle/nakayama/basf2_opt/release_201502_development/Work_MCgen/output/ 2 100 0')
print('will analyse files in output dir with index numbers from 200-299.')
print('Background tag is int appended to all hits (to identify contributions from different sources at later analysis')
print('use 0 for RBB, 1 for BHWide, 2 Touschek_HER, 3 Touschek_LER, 4 Coulomb_HER, 5 Coulomb_LER, 6 2-photon')
print('')

# -------------------------
# here we register modules
# -------------------------

input = register_module('RootInput')
paramloader = register_module('Gearbox')
geobuilder = register_module('Geometry')
back = register_module('ARICHBackground')

# --------------------------------------
# here we set the parameters of modules
# --------------------------------------

typee = sys.argv[1]
path = sys.argv[2]
n = int(sys.argv[3])
nfiles = int(sys.argv[4])
tag = int(sys.argv[5])

fnames = []

# input file naming
# patha = path + typee + '_study-phase2-'
patha = path + typee + '_study-phase3-'

for i in range(n * nfiles, (n + 1) * nfiles):
    filenn = patha + str(i) + '.root'
    fnames.append(filenn)

# output file name
out = 'arich_' + typee + '_' + sys.argv[3] + '_phase2.root'
# out = 'arich_' + typee + '_' + sys.argv[4] + '.root'

print('Output file: ' + out)

input.param('inputFileNames', fnames)
geobuilder.param('components', ['ARICH'])
back.param('FileName', out)
back.param('BkgTag', tag)
# create path
main = create_path()

# add modules to path
main.add_module(input)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(back)

process(main)
