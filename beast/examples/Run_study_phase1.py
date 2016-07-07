#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
import sys
import datetime
from basf2 import *
from subprocess import call

d = datetime.datetime.today()
print((d.strftime('job start: %Y-%m-%d %H:%M:%S\n')))

# parse input
location = ""

# input file path
inputs = str(sys.argv[1]) + "/" + str(sys.argv[2]) + "_" + str(sys.argv[3]) + "*.root"
print(inputs)
# output file path
outfile = str(sys.argv[4]) + "/" + str(sys.argv[2]) + "_" + str(sys.argv[3]) + ".root"
print(outfile)

# Set the global log level
set_log_level(LogLevel.WARNING)
seed = str(sys.argv[5])
print('seed: ', seed)
set_random_seed(int(seed))

ethres = str(sys.argv[6])
print('thres: ', ethres)
erange = str(sys.argv[7])
print('range: ', erange)
sampletime = str(sys.argv[8])
print('sample time: ', sampletime)

# Input
simpleinput = register_module('RootInput')
simpleinput.param('inputFileNames', inputs)

# Output
histo = register_module("HistoManager")  # Histogram Manager
histo.param('histoFileName', outfile)

# suppress info messages during processing:
# set_log_level(LogLevel.WARNING)

# Gearbox
gearbox = register_module('Gearbox')
gearbox.param('fileName', '/geometry/Beast2_phase1.xml')

detector = register_module('QcsmonitorStudy')
detector.param('Ethres', double(ethres))
detector.param('Erange', double(erange))
detector.param('SampleTime', double(sampletime))

# Show progress of processing
progress = register_module('Progress')

# Register necessary modules
main = create_path()
main.add_module(simpleinput)
main.add_module(gearbox)
main.add_module(detector)
main.add_module(histo)
main.add_module(progress)

process(main)

print('Event Statistics:')
print(statistics)

d = datetime.datetime.today()
print(d.strftime('job finish: %Y-%m-%d %H:%M:%S\n'))
