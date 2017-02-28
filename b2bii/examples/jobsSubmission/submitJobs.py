#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This script submit jobs getting the jobs details (runNo, expNo etc)
# from a look-up table which should already exist

# G.Caria

import os
import sys
import csv

# assuming interest only in this type of events
dataType = 'on_resonance'
belleLevel = 'caseB'

yourWorkingDirectoryPath = '.'

# the job we are submitting
myCmd = yourWorkingDirectoryPath + '/analysisJob.sh'

# a bit of clean-up before job submission
# NOTE: assuming these sub-directories exist in yourWorkingDirectoryPath
#       analysisLog/belle/evtgen-charged (and evtgen-mixed)
#       analysisOutput/belle/evtgen-charged (and evtgen-mixed)
# they will be used to store the jobs log files

for eventType in ['evtgen-charged', 'evtgen-mixed']:

    # =================================== Log files ================================

    print('Cleaning log dir ..')
    # path where we want to store the log files
    logDir = yourWorkingDirectoryPath + '/analysisLog/belle/' + eventType

    # clean up log dir before jobs, so only relevant files are saved
    os.system("rm -rf " + logDir + '/*')

    # =================================== Output files  ================================
    print('Cleaning output dir ..')
    # path where we want to store the output files
    outDir = yourWorkingDirectoryPath + '/analysisOutput/belle/' + eventType

    # clean up log dir before jobs, so only relevant files are saved
    os.system('rm -rf ' + outDir + '/*')

# ================================= Submit jobs ! ==============================
print('Jobs submission ..')

# look up table path
txtFile =\
    yourWorkingDirectoryPath + '/belleMClookUpTable.txt'

# open look up table
with open(txtFile) as f:
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
