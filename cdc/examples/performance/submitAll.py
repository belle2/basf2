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
# GCR 2017
dir = '/hsm/belle2/bdata/Data/Raw/' + exp
# pre GCR period
# dir = '/ghi/fs01/belle2/bdata/group/detector/CDC/unpacked/'
# files = glob.glob(dir + exp + '/gcr.cdc.*.root')

# run lists
# runs = [3118, 3119]
runs = [3944, 3948, 3964, 4011, 3966, 3980, 3996, 4002, 4004, 4005]
# runs = [3860, 3871, 3883, 3912, 3917, 3927]
# runs = [3944, 3948, 3964, 4011, 3966,3980,3996,4002,4004,4005,
#        3860, 3871, 3883, 3912, 3917, 3927, 3965, 3967, 3981]
# runs = [i for i in range(3112,4118)]
# print(runs)
# exit(1)


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
