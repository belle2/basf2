#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# G. Caria

import os
import sys
from tools import addLine, getBelleUrl_data, getMaxRunNo_data, readConfigFile_data

# print debug messages ?
debug = False
debugCounter = 0

if len(sys.argv) == 1:
    sys.exit('Need one argument: path of config file with job parameters !')

# read jobs parameters from config file
runsPerJob, expNoList, skimTypeList, dataTypeList, belleLevelList =\
    readConfigFile_data()

# create output directory for tables if it doesn't exist
if not os.path.exists('tables'):
    os.makedirs('tables')

# open table and write in it
# table name contains threshold for events number of each job
tableName = 'tables/lookUpTable_data_' + str(runsPerJob) + '.txt'
f = open(tableName, 'w')

# write one line for each job will submit
# grouping the smallest set of runs that has more than Nthreshold events
for expNo in expNoList:

    absMaxRunNo = getMaxRunNo_data(expNo)
    if absMaxRunNo is None:
        sys.exit('ExpNo ' + str(expNo) + ' not found. Does it exist ?')

    if debug:
        print('For expNo ' + str(expNo) + ' max runNo is ' + str(absMaxRunNo))

    for skimType in skimTypeList:
        for dataType in dataTypeList:
            for belleLevel in belleLevelList:

                minRunNo = 1
                while minRunNo < absMaxRunNo:  # stop searching for runs after maxRunNo

                    maxRunNo = minRunNo + (runsPerJob - 1)

                    thisUrl = getBelleUrl_data(expNo, minRunNo, maxRunNo,
                                               skimType, dataType, belleLevel)

                    if debug:
                        print('Checking up url: ' + thisUrl)

                    addLine(f, [expNo, skimType, dataType,
                                belleLevel, minRunNo, maxRunNo])

                    minRunNo = maxRunNo + 1
f.close()
