#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Functions that help creating a txt file,
# where each line contains the details (expNo, runNo range, etc) of a job to be submitted.
# Runs are grouped according to a min number of events,
# which can be changed through an argument to this script
# and is set by default to 150k events.

# G. Caria

import os
import sys
from tools import *

# print debug messages ?
debug = False
debugCounter = 0

if len(sys.argv) == 1:
    sys.exit('Need one argument: path of config file with job parameters !')

# read jobs parameters from config file
thresholdEventsNo, expNoList, skimTypeList, dataTypeList, belleLevelList =\
    readConfigFile_data()


# open table and write in it
# table name contains threshold for events number of each job
tableName = 'tables/lookUpTable_data_' + str(int(thresholdEventsNo / 1000)) + 'k.txt'
f = open(tableName, 'w')

# write one line for each job will submit
# grouping the smallest set of runs that has more than Nthreshold events
for expNo in expNoList:

    absMaxRunNo = getMaxRunNo(expNo)
    if absMaxRunNo is None:
        sys.exit('ExpNo ' + str(expNo) + ' not found. Does it exist ?')

    if debug:
        print('For expNo ' + str(expNo) + ' max runNo is ' + str(absMaxRunNo))

    for skimType in skimTypeList:
        for dataType in dataTypeList:
            for belleLevel in belleLevelList:

                minRunNo = 1
                while minRunNo < absMaxRunNo:  # stop searching for runs after maxRunNo

                    maxRunNo = minRunNo + 9

                    thisUrl = getBelleUrl_data(expNo, minRunNo, maxRunNo,
                                               skimType, dataType, belleLevel)

                    if debug:
                        print('Checking up url: ' + thisUrl)

                    addLine(f, [expNo, skimType, dataType, belleLevel, minRunNo, maxRunNo])

                    minRunNo = maxRunNo + 1
f.close()
