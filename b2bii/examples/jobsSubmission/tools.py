#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import configparser
import urllib.request
import re
import sys


def readConfigFile_mc():

    config = configparser.ConfigParser()
    config.read(sys.argv[1])

    thresholdEventsNo = int(config['Config']['thresholdEventsNo'])
    expNoList = list(map(int, config['Config']['expNo'].split(',')))
    eventTypeList = list(map(str, config['Config']['eventType'].split(',')))
    dataTypeList = list(map(str, config['Config']['dataType'].split(',')))
    belleLevelList = list(map(str, config['Config']['belleLevel'].split(',')))

    return thresholdEventsNo, expNoList, eventTypeList, dataTypeList, belleLevelList


def readConfigFile_data():

    config = configparser.ConfigParser()
    config.read(sys.argv[1])

    runsPerJob = int(config['Config']['runsPerJob'])
    expNoList = list(map(int, config['Config']['expNo'].split(',')))
    skimTypeList = list(map(str, config['Config']['skimType'].split(',')))
    dataTypeList = list(map(str, config['Config']['dataType'].split(',')))
    belleLevelList = list(map(str, config['Config']['belleLevel'].split(',')))

    return runsPerJob, expNoList, skimTypeList, dataTypeList, belleLevelList


def countEventsInUrl(link):
    aFile = urllib.request.urlopen(link)
    # put webpage content into string format
    fileStr = str(aFile.read(), 'utf-8')
    # get everything after 'Total events' string
    nEventsStr = fileStr.split("Total events: ")[1].split()[0]
    # get the actual number of events
    nEvents = int(re.search(r'\d+', nEventsStr).group())
    return nEvents


def getBelleUrl_mc(expNo, startRun, endRun, eventType, dataType, belleLevel, streamNo):
    header = 'http://bweb3/montecarlo.php?'
    return header +\
        'ex=' + str(expNo) +\
        '&rs=' + str(startRun) +\
        '&re=' + str(endRun) +\
        '&ty=' + eventType +\
        '&dt=' + dataType +\
        '&bl=' + belleLevel +\
        '&st=' + str(streamNo)


def getBelleUrl_data(expNo, startRun, endRun, skimType, dataType, belleLevel):

    header = 'http://bweb3/mdst.php?'
    return header +\
        'ex=' + str(expNo) +\
        '&rs=' + str(startRun) +\
        '&re=' + str(endRun) +\
        '&skm=' + skimType +\
        '&dt=' + dataType +\
        '&bl=' + belleLevel


def addLine(tableFile, aList):
    writeStr = ''
    for el in aList:
        writeStr += str(el) + '\t'
    writeStr += '\n'
    tableFile.write(writeStr)
    print('Added line to table: ' + writeStr)


def getMaxRunNo_mc(expNo):

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


def getMaxRunNo_data(expNo):

    maxRunNoDict = {

        7: 2865,
        9: 1220,
        11: 1367,
        13: 1627,
        15: 1437,
        17: 937,
        19: 1709,
        21: 324,
        23: 607,
        25: 2122,
        27: 1632,
        31: 1715,
        33: 870,
        35: 687,
        37: 1913,
        39: 1357,
        41: 1261,
        43: 1149,
        45: 450,
        47: 881,
        49: 1227,
        51: 1805,
        53: 272,
        55: 1749,
        61: 1373,
        63: 783,
        65: 1232,
        67: 1123,
        69: 1397,
        71: 2292,
        73: 916}

    return int(maxRunNoDict.get(expNo))
