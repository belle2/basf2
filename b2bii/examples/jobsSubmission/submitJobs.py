#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# This script submit jobs getting the jobs details (runNo, expNo etc)
# from a look-up table which should already exist

import os
import sys
import csv


def submit_job(listArgs, logDir, myCmd):
    print(listArgs)
    # end of file name for log files
    filenameEnd = '_'.join(listArgs)

    # job submission with arguments
    os.system('bsub ' +
              ' -q l' +
              ' -oo ' + logDir + '/' + 'out_' + filenameEnd +
              ' -eo ' + logDir + '/' + 'err_' + filenameEnd +
              ' -N' +
              ' \" ' + myCmd +
              ' ' + sys.argv[2].lower() + ' ' +
              ' '.join(listArgs) +
              '\"')

    # summary
    print('Submitted job for: ' + ' '.join(listArgs))


if len(sys.argv) != 3:
    sys.exit('Need two arguments: 1) path of look up table, 2) mc or data')

# yourWorkingDirectoryPath = '.'
# logDir = yourWorkingDirectoryPath + '/analysisLog/'

lookUpTablePath = sys.argv[1]

mc_or_data = sys.argv[2].lower()
isMC = {"mc": True, "data": False}.get(mc_or_data, None)
if isMC is None:
    sys.exit(
        'Second parameter must be "mc" or "data" to indicate whether we run on MC or real data')

# the job we are submitting
# myCmd = yourWorkingDirectoryPath + '/analysisJob.sh'
myCmd = './analysisJob.sh'
logDir = './analysisLog'

# ================================= Submit jobs ! ========================
print('Jobs submission ..')

# open look up table
with open(lookUpTablePath) as f:

    reader = csv.reader(f, delimiter='\t')

    # submit one job for each line , for each streamNo
    for row in reader:
        if isMC:
            print(row)
            expNo = int(row[0])

            if expNo in range(7, 28):
                streamNoList = range(10, 20)
            elif expNo in range(31, 66):
                streamNoList = range(0, 10)

            for streamNo in streamNoList:
                # removing empty element at end of line
                row = row[:-1]
                # adding streamNo
                row.append(str(streamNo))
                submit_job(row, logDir, myCmd)
        else:
            # removing empty element at end of line
            submit_job(row[:-1], logDir, myCmd)
