#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import glob
import subprocess
from functools import reduce
import argparse

parser.add_argument('mode', help='dqm or qam')
args = parser.parse_args()

mode = args.mode


def getRunList(dir):
    files = glob.glob(dir + '/dst.cosmic.*.root')
    runs = []
    #     print(list)
    for f in files:
        run = int((f.split('/'))[-1].split('.')[3])
        runs.append(run)
    #        print(run)

    # Remove duplicated elements, and sort them.
    runs = sorted(list(set(runs)))
    #    print(runs)
    return(runs)


dir = '/ghi/fs01/belle2/bdata/users/karim/data/GCR1/build-2017-08-21'
exp = 1
runs = getRunList(dir)

for run in runs:
    if mode is 'dqm':
        logfile = 'dqm.cosmic.{0:0>4}.{1:0>5}.log'.format(exp, run)
        command = ' '.join(['basf2 CDCHistMaker.py', str(exp), str(run), '>&', logfile])
    elif mode is 'qam':
        logfile = 'qam.cosmic.{0:0>4}.{1:0>5}.log'.format(exp, run)
        command = ' '.join(['basf2 QAMHistMaker.py', str(exp), str(run), '>&', logfile])
    else:
        print('Undefined mode')
        exit(1)

    # Check submitting job list
    # print('bsub -q s \"' + command + ' \"')
    # Submit jobs.
    subprocess.run(["bsub", "-q", "l", command])
