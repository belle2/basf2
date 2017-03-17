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
import re
import urllib.request
import configparser


def countEventsInUrl(link):
    f = urllib.request.urlopen(link)
    # put webpage content into string format
    myfile = str(f.read(), 'utf-8')
    # get everything after 'Total events' string
    nEventsStr = myfile.split("Total events: ")[1].split()[0]
    # get the actual number of events
    nEvents = int(re.search(r'\d+', nEventsStr).group())
    return nEvents


def getBelleMCUrl(expNo, startRun, endRun, eventType, dataType, belleLevel, streamNo):
    header = 'http://bweb3/montecarlo.php?'
    return header +\
        'ex=' + str(expNo) +\
        '&rs=' + str(startRun) +\
        '&re=' + str(endRun) +\
        '&ty=' + eventType +\
        '&dt=' + dataType +\
        '&bl=' + belleLevel +\
        '&st=' + str(streamNo)


def getBelleDataUrl(expNo, startRun, endRun, skimType, dataType, belleLevel):
    header = 'http://bweb3/mdst.php?'
    return header +\
        'ex=' + str(expNo) +\
        '&rs=' + str(startRun) +\
        '&re=' + str(endRun) +\
        '&skm=' + skimType +\
        '&dt=' + dataType +\
        '&bl=' + belleLevel


def addLine(aList):
    writeStr = ''
    for el in aList:
        writeStr += str(el) + '\t'
    writeStr += '\n'
    f.write(writeStr)
    print('Added line to table: ' + writeStr)


def getMaxRunNo(expNo):

    maxRunNoDict = {
        65: 900,
        63: 800,
        61: 1300,
        55: 1800,
        51: 1900,
        49: 1000,
        47: 900,
        45: 500,
        43: 1200,
        41: 1300,
        39: 1400,
        37: 2000,
        35: 700,
        33: 900,
        31: 1800,
        27: 1700,
        25: 2200,
        23: 700,
        21: 400,
        19: 1800,
        17: 1000,
        15: 1500,
        13: 1700,
        11: 1400,
        9: 1300,
        7: 2900}

    return int(maxRunNoDict.get(expNo))

# --------------------------- main function ---------------------------
# print debug messages ?
debug = False

if len(sys.argv) == 1:
    sys.exit('Need one argument: path of config file with job parameters !')

# read jobs parameters from config file
config = configparser.ConfigParser()
config.read(sys.argv[1])
thresholdEventsNo = int(config['Config']['thresholdEventsNo'])
expNoList = list(map(int, config['Config']['expNo'].split(',')))

eventTypeList = ['evtgen-charged', 'evtgen-mixed']
# assuming interest only in this type of events
# otherwise can define lists and then loop on them
dataType = 'on_resonance'
belleLevel = 'caseB'

# open table and write in it
# table name contains threshold for events number of each job
tableName = 'belleMClookUpTable_' + str(int(thresholdEventsNo/1000)) + 'k.txt'
f = open(tableName, 'w')

# write one line for each job will submit
# grouping the smallest set of runs that has more than Nthreshold events
for expNo in expNoList:

    absMaxRunNo = getMaxRunNo(expNo)
    if absMaxRunNo is None:
        sys.exit('ExpNo ' + str(expNo) + ' not found. Does it exist ?')

    if debug:
        print('For expNo ' + str(expNo) + ' max runNo is ' + str(absMaxRunNo))

    if expNo in range(7, 28):
        streamNoList = range(10, 20)
    elif expNo in range(31, 66):
        streamNoList = range(0, 10)

    for eventType in eventTypeList:
        for streamNo in streamNoList:

            minRunNo = 1
            while minRunNo < absMaxRunNo:  # stop searching for runs after maxRunNo

                maxRunNo = minRunNo + 1

                # create the smallest set of runs that has more than Nthreshold events
                for add in range(1, 1000):

                    maxRunNo = minRunNo + add

                    thisUrl = getBelleMCUrl(expNo, minRunNo, maxRunNo,
                                            eventType, dataType, belleLevel, streamNo)
                    if debug:
                        print('checking up url: ' + thisUrl)

                    thisUrlCount = countEventsInUrl(thisUrl)
                    if debug:
                        print('count is up to: ' + str(thisUrlCount))

                    if maxRunNo > absMaxRunNo or thisUrlCount > thresholdEventsNo:
                        break

                addLine([expNo, streamNo, eventType, minRunNo, maxRunNo])

                minRunNo = maxRunNo + 1
f.close()
