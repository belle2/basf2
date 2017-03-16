#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This script submit jobs getting the jobs details (runNo, expNo etc)
# from a look-up table which should already exist

# G.Caria

import os
import sys
import csv

if len(sys.argv) == 1:
    sys.exit('Need one argument: path of look up table !')

# assuming interest only in this type of events
dataType = 'on_resonance'
belleLevel = 'caseB'

yourWorkingDirectoryPath = '.'
subLogDir = yourWorkingDirectoryPath + '/analysisLog/'
lookUpTablePath = yourWorkingDirectoryPath + sys.argv[1]

# the job we are submitting
myCmd = yourWorkingDirectoryPath + '/analysisJob.sh'

# a bit of clean-up before job submission

for eventType in ['evtgen-charged', 'evtgen-mixed']:

    print('Cleaning log dir ..')
    # path where we want to store the log files
    logDir = subLogDir + eventType

    if os.path.exists(logDir):
        # clean up log dir before jobs, so only relevant files are saved
        os.system("rm -rf " + logDir + '/*')
    else:
        os.makedirs(logDir)
# ================================= Submit jobs ! ==============================
print('Jobs submission ..')

# open look up table
with open(lookUpTablePath) as f:
    reader = csv.reader(f, delimiter='\t')

    # for each line of the txt file submit a job
    for row in reader:
        expNo, streamNo, eventType, minRunNo, maxRunNo, nothing = row

        # end of file name for log files
        filenameEnd = str(expNo) + '_' + str(minRunNo) + '_' + str(maxRunNo) +\
            '_' + str(streamNo)

        # job submission with arguments
        os.system('bsub ' +
                  ' -q l' +
                  ' -oo ' + logDir + '/' + 'out_' + filenameEnd +
                  ' -eo ' + logDir + '/' + 'err_' + filenameEnd +
                  ' -N' +
                  ' \" ' + myCmd + ' ' +
                  str(expNo) + ' ' +
                  str(minRunNo) + ' ' +
                  str(maxRunNo) + ' ' +
                  eventType + ' ' +
                  dataType + ' ' +
                  belleLevel + ' ' +
                  str(streamNo) + '\"')

        # summary
        print('Submitted job for: ' +
              str(expNo) + ' ' +
              str(minRunNo) + ' ' +
              str(maxRunNo) + ' ' +
              eventType + ' ' +
              dataType + ' ' +
              belleLevel + ' ' +
              str(streamNo))
