#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import configparser
import urllib.request
import re
import sys
import os


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

    thresholdEventsNo = int(config['Config']['thresholdEventsNo'])
    expNoList = list(map(int, config['Config']['expNo'].split(',')))
    skimTypeList = list(map(str, config['Config']['skimType'].split(',')))
    dataTypeList = list(map(str, config['Config']['dataType'].split(',')))
    belleLevelList = list(map(str, config['Config']['belleLevel'].split(',')))

    return thresholdEventsNo, expNoList, skimTypeList, dataTypeList, belleLevelList


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
