#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import glob
import subprocess
from functools import reduce

'''
Simple script submitting jobs to process all data.
Usage:
basf2 submitAll.py
'''

# Experimantl number
exp = 'e0001'
# Data directory
# pre GCR period
# dir = '/ghi/fs01/belle2/bdata/group/detector/CDC/unpacked/'
# files = glob.glob(dir + exp + '/gcr.cdc.*.root')
# GCR 2017
dir = '/hsm/belle2/bdata/Data/Raw/' + exp
# run lists
runs = [3118, 3119, 3120, 3165, 3169, 3170, 3173]
files_nested = [glob.glob(dir + '/*/sub00/cosmic.*' + str(i) + '.HLT1.f*.root') for i in runs]
# flatting nested list.
files = reduce(lambda x, y: x + y, files_nested)

if not files:
    print("No data are found.")
    quit()

for f in files:
    input = f  # input file name.
    name = f.split('/')
    logfile = name[-1].replace('.root', '.log')  # input file name.
    output = 'output_' + name[-1]  # output file name.
    command = ' '.join(['basf2 runReconstruction.py', input, output, '>&', logfile])
    # Check submitting job list
    # print('bsub -q s \"' + command + ' \"')
    # Submit jobs.
    subprocess.run(["bsub", "-q", "l", command])
