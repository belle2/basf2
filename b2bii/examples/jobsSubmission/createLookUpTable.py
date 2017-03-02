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


def addLine(aList):
    writeStr = ''
    for el in aList:
        writeStr += str(el) + '\t'
    writeStr += '\n'
    f.write(writeStr)
    print('Added line to table: ' + writeStr)

# --------------------------- main function ---------------------------

# change min number of events per job here
if len(sys.argv) > 1:
    thresholdEventsNo = int(sys.argv[1])
else:
    thresholdEventsNo = 150000

# assuming interest only in this type of events
# otherwise can define ranges and then loop on them
expNo = 65
dataType = 'on_resonance'
belleLevel = 'caseB'

# max for run number for exp number 65
# can define dictionary (expNo: maxRunNo) for all exp numbers
absMaxRunNo = 850

f = open('belleMClookUpTable.txt', 'w')

# write one line for each job will submit
# grouping the smallest set of runs that has more than N events
for eventType in ['evtgen-charged', 'evtgen-mixed']:
    for streamNo in range(0, 10):

        minRunNo = 1
        while minRunNo < absMaxRunNo:  # stop searching for runs after maxRunNo

            maxRunNo = minRunNo + 1

            # create the smallest set of runs that has more than N events
            for add in range(1, 1000):

                maxRunNo = minRunNo + add

                thisUrl = getBelleMCUrl(expNo, minRunNo, maxRunNo,
                                        eventType, dataType, belleLevel, streamNo)

                thisUrlCount = countEventsInUrl(thisUrl)

                if maxRunNo > absMaxRunNo or thisUrlCount > thresholdEventsNo:
                    break

            addLine([expNo, streamNo, eventType, minRunNo, maxRunNo])

            minRunNo = maxRunNo + 1
f.close()
