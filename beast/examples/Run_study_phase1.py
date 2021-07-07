#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import sys
import datetime
import basf2 as b2

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
b2.set_log_level(b2.LogLevel.WARNING)
seed = str(sys.argv[5])
print('seed: ', seed)
b2.set_random_seed(int(seed))

ethres = str(sys.argv[6])
print('thres: ', ethres)
erange = str(sys.argv[7])
print('range: ', erange)
sampletime = str(sys.argv[8])
print('sample time: ', sampletime)

# Input
simpleinput = b2.register_module('RootInput')
simpleinput.param('inputFileNames', inputs)

# Output
histo = b2.register_module("HistoManager")  # Histogram Manager
histo.param('histoFileName', outfile)

# suppress info messages during processing:
# set_log_level(LogLevel.WARNING)

# Gearbox
gearbox = b2.register_module('Gearbox')
gearbox.param('fileName', '/geometry/Beast2_phase1.xml')

detector = b2.register_module('QcsmonitorStudy')
detector.param('Ethres', float(ethres))
detector.param('Erange', float(erange))
detector.param('SampleTime', float(sampletime))

# Show progress of processing
progress = b2.register_module('Progress')

# Register necessary modules
main = b2.create_path()
main.add_module(simpleinput)
main.add_module(gearbox)
main.add_module(detector)
main.add_module(histo)
main.add_module(progress)

b2.process(main)

print('Event Statistics:')
print(b2.statistics)

d = datetime.datetime.today()
print(d.strftime('job finish: %Y-%m-%d %H:%M:%S\n'))
