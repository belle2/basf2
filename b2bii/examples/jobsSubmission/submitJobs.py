#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This script submit jobs getting the jobs details (runNo, expNo etc)
# from a look-up table which should already exist

# G.Caria

import os
import sys
import csv

if len(sys.argv) != 3:
    sys.exit('Need two arguments: 1) path of look up table, 2) mc or data')

# yourWorkingDirectoryPath = '.'
# logDir = yourWorkingDirectoryPath + '/analysisLog/'

lookUpTablePath = sys.argv[1]

mc_or_data = sys.argv[2].lower()
isMC = {"mc": True, "data": False}.get(mc_or_data, None)
if isMC is None:
    sys.exit('Second parameter must be "mc" or "data" to indicate whether we run on MC or real data')

# the job we are submitting
# myCmd = yourWorkingDirectoryPath + '/analysisJob.sh'
myCmd = './analysisJob.sh'
logDir = './analysisLog'

# ================================= Submit jobs ! ==============================
print('Jobs submission ..')

# open look up table
with open(lookUpTablePath) as f:
    reader = csv.reader(f, delimiter='\t')

    # for each line of the txt file submit a job
    for row in reader:

        if isMC:
            expNo, eventType, streamNo, dataType, belleLevel, minRunNo, maxRunNo =\
                row[:-1]
        else:
            expNo, skimType, dataType, belleLevel, minRunNo, maxRunNo =\
                row[:-1]

        # end of file name for log files
        filenameEnd = '_'.join(row[:-1])

        # job submission with arguments
        os.system('bsub ' +
                  ' -q l' +
                  ' -oo ' + logDir + '/' + 'out_' + filenameEnd +
                  ' -eo ' + logDir + '/' + 'err_' + filenameEnd +
                  ' -N' +
                  ' \" ' + myCmd +
                  ' ' + sys.argv[2].lower() + ' ' +
                  ' '.join(row[:-1]) +
                  '\"')
        print('bsub ' +
              ' -q l' +
              ' -oo ' + logDir + '/' + 'out_' + filenameEnd +
              ' -eo ' + logDir + '/' + 'err_' + filenameEnd +
              ' -N' +
              ' \" ' + myCmd + ' ' +
              ' ' + sys.argv[2].lower() + ' ' +
              ' '.join(row[:-1]) +
              '\"')

        # summary
        print('Submitted job for: ' + ' '.join(row))
