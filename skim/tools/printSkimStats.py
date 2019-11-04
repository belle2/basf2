#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""

"""


__author__ = "Phil Grace, Racha Cheaib"
__email__ = "philip.grace@adelaide.edu.au, rachac@mail.ubc.ca"


import json
from os.path import getsize
# import pandas as pd
import re


skims = [
    'LeptonicUntagged',
]

dataLabels = ['MC12_mixedBGx1', 'MC12_chargedBGx1', 'MC12_ccbarBGx1', 'MC12_ssbarBGx1',
              'MC12_uubarBGx1', 'MC12_ddbarBGx1', 'MC12_taupairBGx1',
              'MC12_mixedBGx0', 'MC12_chargedBGx0', 'MC12_ccbarBGx0', 'MC12_ssbarBGx0',
              'MC12_uubarBGx0', 'MC12_ddbarBGx0', 'MC12_taupairBGx0']


def divide(numerator, denominator):
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
        skimStats = {
            'RetentionRate': divide(self.nSkimmedEvents(), self.nInputEvents()),
            'nInputEvents': self.nInputEvents(),
            'nSkimmedEvents': self.nSkimmedEvents(),
            'cpuTime': self.cpuTime(),
            'cpuTimePerEvent': divide(self.cpuTime(), self.nInputEvents()),
            'udstSize': divide(self.udstSize(), 1024),
            'udstSizePerEvent': divide(self.udstSize(), self.nInputEvents()),
            'logSize': divide(self.logSize(), 1024),
            'logSizePerEvent': divide(self.logSize(), self.nInputEvents()),
            'memoryAverage': self.memoryAverage(),
            'memoryMaximum': self.memoryMaximum(),
            # TODO: Calculate these:
            'udstSizePerEntireSample': None,
            'logSizePerEntireSample': None,
        }

        return skimStats

    def nInputEvents(self):
        return {l: self.__json[l]['basf2_status']['total_events'] for l in self.__labels}

    def nSkimmedEvents(self):
        return {l: self.__json[l]['output_files'][0]['stats']['events'] for l in self.__labels}

    def udstSize(self):
        """Return the size of the output uDST file in KB, read from Job Information JSON file."""
        return {l: self.__json[l]['output_files'][0]['stats']['filesize_kib'] for l in self.__labels}

    def logSize(self):
        """Return the size of the log file in KB, directly measured from output file."""
        return {l: getsize(self.__logFileNames[l]) / 1024 for l in self.__labels}

    def cpuTime(self):
        return {l: self.getStatFromLog('CPU time', l) for l in self.__labels}

    def memoryAverage(self):
        return {l: self.getStatFromLog('Average Memory', l) for l in self.__labels}

    def memoryMaximum(self):
        return {l: self.getStatFromLog('Max Memory', l) for l in self.__labels}

    def nTotalEvents(self):
        return {l: get_total_infiles(l, self.__skimName) for l in self.__labels}

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
