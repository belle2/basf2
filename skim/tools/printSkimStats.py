#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""

"""


__author__ = "Phil Grace, Racha Cheaib"
__email__ = "philip.grace@adelaide.edu.au, rachac@mail.ubc.ca"


from functools import lru_cache
import json
from os.path import getsize
import re

# from skimExpertFunctions import get_eventN, get_total_infiles

skims = [
    'LeptonicUntagged',
]

dataLabels = ['MC12_mixedBGx1', 'MC12_chargedBGx1', 'MC12_ccbarBGx1', 'MC12_ssbarBGx1',
              'MC12_uubarBGx1', 'MC12_ddbarBGx1', 'MC12_taupairBGx1',
              'MC12_mixedBGx0', 'MC12_chargedBGx0', 'MC12_ccbarBGx0', 'MC12_ssbarBGx0',
              'MC12_uubarBGx0', 'MC12_ddbarBGx0', 'MC12_taupairBGx0']


def dictDivide(numerator, denominator):
    """Divide two dicts elementwise, or divide all elements of a dict by a float or int.

    Args:
        numerator (dict):
        denominator (dict, float, int):
    """
    try:
        assert numerator.keys() == denominator.keys()
        dividedDict = {label: n / d for (label, n), (_, d) in zip(numerator.items(), denominator.items())}
    except AttributeError or TypeError:
        dividedDict = {label: n / denominator for label, n in numerator.items()}

    return dividedDict


def dictTimes(dict1, multiplier):
    """Multiply two dicts elementwise, or multiply all elements of a dict by a float or int.

    Args:
        dict1 (dict):
        multiplier (dict, float, int):
    """
    try:
        assert dict1.keys() == multiplier.keys()
        multipliedDict = {label: v1 * v2 for (label, v1), (_, v2) in zip(dict1.items(), multiplier.items())}
    except AttributeError or TypeError:
        multipliedDict = {label: v * multiplier for label, n in dict1.items()}

    return multipliedDict


class SkimStats:
    """Class for managing the skim stats of a single skim."""
    # TODO: put this in with getSkimStats().
    columnLabels = {
        'RetentionRate': 'Retention rate',
        'nInputEvents': 'Number of input events of test',
        'nSkimmedEvents': 'Number of skimmed events',
        'timePerEvent': 'CPU time per event',
        'cpuTime': 'CPU time of test',
        'udstSizePerEvent': 'uDST size per event (kB)',
        'udstSize': 'uDST size of test (MB)',
        'logSizePerEvent': 'Log size per event (kB)',
        'logSize': 'Log size of test (MB)',
        'memoryAverage': 'Average memory usage (MB)',
        'memoryMaximum': 'Maximum memory usage (MB)',
        'udstSizePerEntireSample': 'Estimated uDST size for entire sample (GB)',
        'logSizePerEntireSample': 'Estimated log size for entire sample (GB)'
    }

    def __init__(self, skimName, dataLabels):
        self.__skimName = skimName
        self.__labels = dataLabels

        self.__jsonFileNames = {l: f'log/JobInformation_{self.__skimName}_{l}.json' for l in self.__labels}
        self.__logFileNames = {l: f'log/{self.__skimName}_{l}.out' for l in self.__labels}

        self.__json = self.loadJson()
        self.__logs = self.loadLogs()

    def loadLogs(self):
        """"""
        logFileContents = {}

        for label in self.__labels:
            with open(self.__logFileNames[label], 'r') as logFile:
                logFileContents[label] = logFile.read()

        return logFileContents

    def loadJson(self):
        """Read in the job information JSON files produced when passing `--job-information` flag to
        basf2.
        """
        # TODO: make this in the log/ directory
        statsJson = {}

        for label in self.__labels:
            with open(self.__jsonFileNames[label]) as jsonFile:
                statsJson[label] = json.load(jsonFile)

        return statsJson

    def statsDict(self):
        nInputEvents = self.makeDict(self.nInputEvents)
        nSkimmedEvents = self.makeDict(self.nSkimmedEvents)
        udstSize = self.makeDict(self.udstSize)
        logSize = self.makeDict(self.logSize)
        cpuTime = self.makeDict(self.cpuTime)
        memoryAverage = self.makeDict(self.memoryAverage)
        memoryMaximum = self.makeDict(self.memoryMaximum)

        nEventsPerFile = self.makeDict(self.nEventsPerFile)
        nTotalFiles = self.makeDict(self.nEventsPerFile)
        nTotalEvents = dictMult(self.__nEventsPerFile, self.__nTotalFiles)

        skimStats = {
            'RetentionRate': dictDivide(nSkimmedEvents, nInputEvents),
            'nInputEvents': nInputEvents,
            'nSkimmedEvents': nSkimmedEvents,
            'cpuTime': cpuTime,
            'cpuTimePerEvent': dictDivide(cpuTime, nInputEvents),
            'udstSize': dictDivide(udstSize, 1024),
            'udstSizePerEvent': dictDivide(udstSize, nInputEvents),
            'logSize': dictDivide(logSize, 1024),
            'logSizePerEvent': dictDivide(logSize, nInputEvents),
            'memoryAverage': memoryAverage,
            'memoryMaximum': memoryMaximum,
            # TODO: Calculate these:
            'udstSizePerEntireSample': dictMult(dictDivide(udstSize, nInputEvents), nTotalEvents),
            'logSizePerEntireSample': None,
        }

        return skimStats

    def makeDict(self, func):
        return {l: func(l) for l in self.__labels}

    def nInputEvents(self, dataLabel):
        json = self.__json[dataLabel]
        return json['basf2_status']['total_events']

    def nSkimmedEvents(self, dataLabel):
        json = self.__json[dataLabel]
        return json['output_files'][0]['stats']['events']

    def udstSize(self, dataLabel):
        """Return the size of the output uDST file in KB, read from Job Information JSON file."""
        json = self.__json[dataLabel]
        return json['output_files'][0]['stats']['filesize_kib']

    def logSize(self, dataLabel):
        """Return the size of the log file in KB, directly measured from output file."""
        logFileName = self.__logFileNames[dataLabel]
        return getsize(logFileName) / 1024

    def cpuTime(self, dataLabel):
        return self.getStatFromLog('CPU time', dataLabel)

    def memoryAverage(self, dataLabel):
        return self.getStatFromLog('Average Memory', dataLabel)

    def memoryMaximum(self, dataLabel):
        return self.getStatFromLog('Max Memory', dataLabel)

    def nEventsPerFile(self, dataLabel):
        parentFile = get_test_file(dataLabel)
        return get_eventN(parentFile)

    def nTotalFiles(self, dataLabel):
        return get_total_infiles(dataLabel, self.__skimName)

    # Helper functions
    def getStatFromLog(self, statistic, dataLabel):
        """Search for a given statistic in the "Resource usage summary" section of the log file."""
        logFileContents = self.__logs[dataLabel]

        floatRegexp = '\s*:\s+(\d+(\.(\d+)?)?)'
        statFromLog = re.findall(f'{statistic}{floatRegexp}', logFileContents)[0][0]

        # TODO Error handling for when it can't find a match.
        return float(statFromLog)


if __name__ == '__main__':

    allSkimStats = {skim: SkimStats(skim, dataLabels).statsDict() for skim in skims}
    print(allSkimStats)

    outputJsonName = 'skimStats.json'
    with open(outputJsonName, 'w') as outputJson:
        json.dump(allSkimStats, outputJson, indent=4)
