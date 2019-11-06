#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""

"""


__author__ = "Phil Grace, Racha Cheaib"
__email__ = "philip.grace@adelaide.edu.au, rachac@mail.ubc.ca"


import argparse
from functools import lru_cache
import json
from os.path import getsize
import pandas as pd
import re
from textwrap import wrap

from tabulate import tabulate


# from skimExpertFunctions import get_test_file, get_eventN, get_total_infiles


skims = [
    'LeptonicUntagged',
]

beamBackgroundWeights = {
    'BGx1': 0.8,
    'BGx0': 0.2
}

mcSampleCrossSections = {
    'mixed': 0.555,
    'charged': 0.555,
    'ccbar': 1.3,
    'uubar': 1.61,
    'ddbar': 0.40,
    'ssbar': 0.38,
    'taupair': 0.91
}

mcCampaign = 'MC12'

mcSamples = [f'{mcCampaign}_{mcSample}{beamBackground}'
             for beamBackground in beamBackgroundWeights.keys()
             for mcSample in mcSampleCrossSections.keys()]

dataSamples = []
# TODO: acquire data samples and put in registry
# dataSamples = ['proc9_exp3', 'proc9_exp7', 'proc9_exp8', 'bucket7_exp8']

sampleLabels = mcSamples + dataSamples


def getStatFromLog(statistic, logFileContents):
    """Search for a given statistic in the "Resource usage summary" section of the log file."""
    floatRegexp = '\s*:\s+(\d+(\.(\d+)?)?)'
    statFromLog = re.findall(f'{statistic}{floatRegexp}', logFileContents)[0][0]

    return float(statFromLog)


def nInputEvents(json):
    return json['basf2_status']['total_events']


def nSkimmedEvents(json):
    return json['output_files'][0]['stats']['events']


def udstSize(json):
    """Return the size of the output uDST file in KB, read from Job Information JSON file."""
    return json['output_files'][0]['stats']['filesize_kib']


def logSize(log):
    """Return the size of the log file in KB."""
    return len(log) / 1024


def cpuTime(log):
    return getStatFromLog('CPU time', log)


def memoryAverage(log):
    return getStatFromLog('Average Memory', log)


def memoryMaximum(log):
    return getStatFromLog('Max Memory', log)


# TODO: copy previous method of calculating this.
def candidateMultiplicity(log):
    return


# @lru_cache()
# def nEventsPerFile(sampleLabel):
#     parentFile = get_test_file(sampleLabel)
#     return get_eventN(parentFile)


# def nTotalFiles(sampleLabel):
#     return get_total_infiles(sampleLabel)


# def nTotalEvents(sampleLabel):
#     return nEventsPerFile(sampleLabel)*nTotalFiles(sampleLabel)


def mcWeightedAverage(statsPerSample):
    totalCrossSection = sum(mcSampleCrossSections.values())

    weightedAverage = 0
    for mcSample, crossSection in mcSampleCrossSections.items():
        for beamBackground, beamBackgroundWeight in beamBackgroundWeights.items():
            sampleLabel = f'{mcCampaign}_{mcSample}{beamBackground}'
            weightedAverage += statsPerSample[sampleLabel]*beamBackgroundWeight*crossSection/totalCrossSection

    return weightedAverage

statistics = {
    'RetentionRate': {
        'LongName': 'Retention rate (%)',
        'floatfmt': '.2f',
        'Calculate': lambda json, log, skim, sample: 100 * nSkimmedEvents(json) / nInputEvents(json),
        'Combine': lambda statDict: mcWeightedAverage(statDict)
    },
    'nInputEvents': {
        'LongName': 'Number of input events of test',
        'floatfmt': '.0f',
        'Calculate': lambda json, log, skim, sample: nInputEvents(json),
        'Combine': None
    },
    'nSkimmedEvents': {
        'LongName': 'Number of skimmed events',
        'floatfmt': '.0f',
        'Calculate': lambda json, log, skim, sample: nSkimmedEvents(json),
        'Combine': None
    },
    'cpuTime': {
        'LongName': 'CPU time of test on KEKCC (s)',
        'floatfmt': '.1f',
        'Calculate': lambda json, log, skim, sample: cpuTime(log),
        'Combine': None
    },
    'cpuTimePerEvent': {
        'LongName': 'CPU time per event on KEKCC (s)',
        'floatfmt': '.3f',
        'Calculate': lambda json, log, skim, sample: cpuTime(log) / nInputEvents(json),
        'Combine': lambda statDict: mcWeightedAverage(statDict)
    },
    'udstSize': {
        'LongName': 'uDST size of test (MB)',
        'floatfmt': '.2f',
        'Calculate': lambda json, log, skim, sample: udstSize(json) / 1024,
        'Combine': None
    },
    'udstSizePerEvent': {
        'LongName': 'uDST size per event (kB)',
        'floatfmt': '.3f',
        'Calculate': lambda json, log, skim, sample: udstSize(json) / nInputEvents(json),
        'Combine': lambda statDict: mcWeightedAverage(statDict)
    },
    'logSize': {
        'LongName': 'Log size of test (kB)',
        'floatfmt': '.1f',
        'Calculate': lambda json, log, skim, sample: logSize(log),
        'Combine': None
    },
    'logSizePerEvent': {
        'LongName': 'Log size per event (B)',
        'floatfmt': '.2f',
        'Calculate': lambda json, log, skim, sample: logSize(log) / nInputEvents(json) * 1024,
        'Combine': lambda statDict: mcWeightedAverage(statDict)
    },
    'memoryAverage': {
        'LongName': 'Average memory usage (MB)',
        'floatfmt': '.0f',
        'Calculate': lambda json, log, skim, sample: memoryAverage(log),
        'Combine': lambda statDict: mcWeightedAverage(statDict)
    },
    'memoryMaximum': {
        'LongName': 'Maximum memory usage (MB)',
        'floatfmt': '.0f',
        'Calculate': lambda json, log, skim, sample: memoryMaximum(log),
        'Combine': lambda statDict: max(statDict.values())
    },
    # 'udstSizePerEntireSample': {
    #     'LongName': 'Estimated uDST size for entire sample (GB)',
    #     'floatfmt': '.2f',
    #     'Calculate': lambda json, log, skim, sample: udstSize(json) * nTotalEvents(sample) / nInputEvents(json) / 1024,
    #     'Combine': lambda statDict: sum(statDict.values())
    # },
    # 'logSizePerEntireSample': {
    #     'LongName': 'Estimated log size for entire sample (GB)}',
    #     'floatfmt': '.2f',
    #     'Calculate': lambda json, log, skim, sample: logSize(log) * nTotalEvents(sample) / nInputEvents(json) / 1024,
    #     'Combine': lambda statDict: sum(statDict.values())
    # }
}


def toJson(allSkimStats):
    with open('skimStats.json', 'w') as outputJson:
        json.dump(allSkimStats, outputJson, indent=4)


def toScreen(allSkimStats):
    for skimName, skimStats in allSkimStats.items():
        print(f'Performance statistics for {skimName} skim:')

        # Only print some stats to screen
        printingStats = [stat for (stat, statInfo) in statistics.items() if statInfo['Combine']]
        df = pd.DataFrame(skimStats, columns=printingStats)

        headers = ['\n'.join(wrap(statistics[stat]['LongName'], 12)) for stat in printingStats]
        floatfmt = [''] + [statistics[stat]['floatfmt'] for stat in printingStats]

        table = tabulate(df[printingStats],
                         headers=headers, tablefmt="fancy_grid",
                         numalign='right', stralign='left',
                         floatfmt=floatfmt)
        table = table.replace(' nan ', ' -   ')
        print(table)


def toConfluence(allSkimStats):
    confluenceStrings = []
    for skimName, skimStats in allSkimStats.items():
        confluenceStrings += [f'h1. Performance statistics for {skimName} skim']
        df = pd.DataFrame(skimStats)

        headers = [statistics[stat]['LongName'] for stat in statistics]
        floatfmt = [''] + [statistics[stat]['floatfmt'] for stat in statistics]

        table = tabulate(df, headers=headers, tablefmt="jira", floatfmt=floatfmt)

        # Make the first column (the sample label) bold on Confluence
        table = re.sub(r'^\| ', '|| ', table, flags=re.MULTILINE)
        table = table.replace(' nan ', ' -- ')

        confluenceStrings += [table]

    confluenceString = '\n'.join(confluenceStrings)
    with open('skimStats_confluence.txt', 'w') as confluenceFile:
        confluenceFile.write(confluenceString)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--screen', action='store_true', default=False,
                        help='Provide this flag to print stats to screen.')
    parser.add_argument('--confluence', action='store_true', default=False,
                        help='Provide this flag to produce text file output for copying to Confluence.')
    parser.add_argument('--combined', action='store_true', default=False,
                        help='Provide this flag if running the combined skims.')
    args = parser.parse_args()

    # TODO: put into function getSkimStats(skims) which returns a dict
    allSkimStats = {skim: {stat: {} for stat in statistics.keys()} for skim in skims}

    for skim in skims:
        for sampleLabel in sampleLabels:
            logFileName = f'log/{skim}_{sampleLabel}.out'
            with open(logFileName) as logFile:
                logContents = logFile.read()

            jsonFileName = f'log/JobInformation_{skim}_{sampleLabel}.json'
            with open(jsonFileName) as jsonFile:
                jsonContents = json.load(jsonFile)

            for statName, statInfo in statistics.items():
                statFunction = statInfo['Calculate']

                allSkimStats[skim][statName][sampleLabel] = statFunction(jsonContents, logContents, skim, sampleLabel)

    toJson(allSkimStats)

    for skim in skims:
        for statName, statInfo in statistics.items():
            try:
                combiningFunction = statInfo['Combine']
                allSkimStats[skim][statName]['Combined MC'] = combiningFunction(allSkimStats[skim][statName])
            except TypeError:
                allSkimStats[skim][statName]['Combined MC'] = None

    if args.screen:
        toScreen(allSkimStats)
    if args.confluence:
        toConfluence(allSkimStats)
