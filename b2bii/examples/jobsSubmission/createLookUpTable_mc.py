#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# G. Caria

import os
import sys
from tools import getMaxRunNo_mc, getBelleUrl_mc, readConfigFile_mc, countEventsInUrl, addLine

# print debug messages ?
debug = False
debugCounter = 0

if len(sys.argv) == 1:
    sys.exit('Need one argument: path of config file with job parameters !')

# read jobs parameters from config file
thresholdEventsNo, expNoList, eventTypeList, dataTypeList, belleLevelList =\
    readConfigFile_mc()

# create output directory for tables if it doesn't exist
if not os.path.exists('tables'):
    os.makedirs('tables')

# open table and write in it
# table name contains threshold for events number of each job
tableName = 'tables/lookUpTable_mc_' + \
    str(int(thresholdEventsNo / 1000)) + 'k.txt'
f = open(tableName, 'w')

# write one line for each job we submit
# grouping the smallest set of runs that has more than Nthreshold events
for expNo in expNoList:

    absMaxRunNo = getMaxRunNo_mc(expNo)
    if absMaxRunNo is None:
        sys.exit('ExpNo ' + str(expNo) + ' not found. Does it exist ?')

    if debug:
        print('For expNo ' + str(expNo) + ' max runNo is ' + str(absMaxRunNo))

    if expNo in range(7, 28):
        streamNo_dummy = 10
    elif expNo in range(31, 66):
        streamNo_dummy = 0

    for eventType in eventTypeList:
        for dataType in dataTypeList:
            for belleLevel in belleLevelList:

                minRunNo = 1
                while minRunNo < absMaxRunNo:  # stop searching for runs after maxRunNo

                    maxRunNo = minRunNo + 1

                    # create the smallest set of runs that has more than
                    # Nthreshold events
                    for add in range(1, 1000):

                        maxRunNo = minRunNo + add

                        thisUrl = getBelleUrl_mc(expNo, minRunNo, maxRunNo,
                                                 eventType, dataType,
                                                 belleLevel, streamNo_dummy)

                        if debug:
                            print('Checking up url: ' + thisUrl)

                        thisUrlCount = countEventsInUrl(thisUrl)
                        if debug:
                            print('Count is up to: ' + str(thisUrlCount))

                        if maxRunNo > absMaxRunNo or thisUrlCount > thresholdEventsNo:
                            break

                    addLine(f, [expNo, eventType,
                                dataType, belleLevel,
                                minRunNo, maxRunNo])

                    debugCounter += 1
                    if debug and debugCounter > 2:
                        f.close()
                        sys.exit('Debug')

                    minRunNo = maxRunNo + 1

f.close()
