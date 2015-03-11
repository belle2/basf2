#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# This is example script to run extract ARICH background relevant data
# from prepared background samples. In runs ARICHBackground module and
# produces root file with TTree, needed to produce bakcground plots.
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

print ''
print 'Use the script as: basf2 ARICHBkg.py arguments'
print 'Arguments are in the following order: type (RBB_HER,Touschek_HER,...), path to files , job number , number of files to analyse'
print ''
print 'example: basf2 ARICHBkg.py RBB_HER /gpfs/home/belle/nakayama/basf2_opt/release_201502_development/Work_MCgen/output/ 0 100'
print 'will analyse first hundred RBB_HER files'
print ''

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
fnames = []

patha = path + 'output_' + typee + '_study_'

for i in range(nfiles * n, nfiles * (n + 1)):
    filenn = patha + str(i) + '.root'
    fnames.append(filenn)

out = 'arich_' + typee + '_' + str(n) + '.root'
print 'Output file: ' + out

input.param('inputFileNames', fnames)
geobuilder.param('components', ['ARICH'])
back.param('FileName', out)

# create path
main = create_path()

# add modules to path
main.add_module(input)
main.add_module(paramloader)
main.add_module(geobuilder)
main.add_module(back)

process(main)
