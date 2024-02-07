#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import glob
import subprocess
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('type', help='dqm or qam', type=str)
args = parser.parse_args()

mode = args.type


def getRunList(dir):
    files = glob.glob(dir + '*/all/dst/sub00/dst.cosmic.*.root')
#    files = glob.glob(dir + 'run_*.root')
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


# Experimantl number
exp = 1
# dir = '/ghi/fs01/belle2/bdata/users/karim/data/GCR1/build-2017-08-21'
dir = f'/hsm/belle2/bdata/Data/release-00-09-01/DB00000266/GCR1/prod00000001/e{exp:0>4}/cosmics/'
# dir = '/ghi/fs01/belle2/bdata/users/karim/MC/GCR1/release-00-09-02'
# dir = '/ghi/fs01/belle2/bdata/users/karim/data/GCR1/release-00-09-00_new'
# dir = '/home/belle/muchida/basf2/release/cdc/examples/performance/output'

runs = getRunList(dir)
# runs = [i for i in range(4000,4200)]

for run in runs:
    if mode == 'dqm':
        logfile = f'dqm.cosmic.{exp:0>4}.{run:0>5}.log'
        command = ' '.join(['basf2 CDCHistMaker.py', str(exp), str(run), '>&', logfile])
    elif mode == 'qam':
        logfile = f'qam.cosmic.{exp:0>4}.{run:0>5}.log'
        command = ' '.join(['basf2 QAMHistMaker.py', str(exp), str(run), '>&', logfile])
    else:
        print('Specify the type : dqm/qam')
        exit(1)

    # Check submitting job list
    # print('bsub -q s \"' + command + ' \"')
    # Submit jobs.
    subprocess.run(["bsub", "-q", "l", command])
