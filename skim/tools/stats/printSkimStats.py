#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""

"""


__author__ = "Phil Grace, Racha Cheaib"
__email__ = "philip.grace@adelaide.edu.au, rachac@mail.ubc.ca"


import argparse
from functools import lru_cache
import json
import pandas as pd
from pathlib import Path
import re
import sys
from textwrap import wrap

from ROOT import PyConfig
from tabulate import tabulate

# Importing ROOT in skimExpertFunctions has the side-effect of hijacking argument parsing
PyConfig.IgnoreCommandLineOptions = True
from skim.registry import skim_registry, combined_skims
from skimExpertFunctions import get_test_file, get_eventN, get_total_infiles, get_events_per_file


allStandaloneSkims = [skim for _, skim in skim_registry]
allCombinedSkims = list(combined_skims.keys())

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

mcSamples = {
    f'{mcCampaign}_mixedBGx1': f'{mcCampaign}: mixed BGx1',
    f'{mcCampaign}_chargedBGx1': f'{mcCampaign}: charged BGx1',
    f'{mcCampaign}_ccbarBGx1': f'{mcCampaign}: ccbar BGx1',
    f'{mcCampaign}_uubarBGx1': f'{mcCampaign}: uubar BGx1',
    f'{mcCampaign}_ddbarBGx1': f'{mcCampaign}: ddbar BGx1',
    f'{mcCampaign}_ssbarBGx1': f'{mcCampaign}: ssbar BGx1',
    f'{mcCampaign}_taupairBGx1': f'{mcCampaign}: taupair BGx1',
    f'{mcCampaign}_mixedBGx0': f'{mcCampaign}: mixed BGx0',
    f'{mcCampaign}_chargedBGx0': f'{mcCampaign}: charged BGx0',
    f'{mcCampaign}_ccbarBGx0': f'{mcCampaign}: ccbar BGx0',
    f'{mcCampaign}_uubarBGx0': f'{mcCampaign}: uubar BGx0',
    f'{mcCampaign}_ddbarBGx0': f'{mcCampaign}: ddbar BGx0',
    f'{mcCampaign}_ssbarBGx0': f'{mcCampaign}: ssbar BGx0',
    f'{mcCampaign}_taupairBGx0': f'{mcCampaign}: taupair BGx0'
}

dataSamples = {
    'proc9_exp3': 'Data: proc9 exp. 3',
    'proc9_exp7': 'Data: proc9 exp. 7',
    'proc9_exp8': 'Data: proc9 exp. 8',
    'bucket7_exp8': 'Data: bucket7 exp. 8'
}

samples = list(mcSamples.keys()) + list(dataSamples.keys())


class SkimNotRunError(Exception):
    pass


def getStatFromLog(statistic, logFileContents):
    """Search for a given statistic in the "Resource usage summary" section of the log file."""
    floatRegexp = r'\s*:\s+(\d+(\.(\d+)?)?)'
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
    """Return the size of the log file in kB."""
    return len(log) / 1024


def cpuTime(log):
    return getStatFromLog('CPU time', log)


def averageCandidateMultiplicity(log):
    floatRegexp = r'\d+\.\d+'

    multiplicityBlock = re.findall('(Average Candidate Multiplicity(.*\n)+?.*INFO)', log)[0][0]
    multiplicityLines = [line for line in multiplicityBlock.split('\n') if re.findall(floatRegexp, line)]
    multiplicities = [float(re.findall(floatRegexp, line)[1]) for line in multiplicityLines]

    averageMultiplicity = sum(multiplicities)/len(multiplicities)

    return averageMultiplicity


def memoryAverage(log):
    return getStatFromLog('Average Memory', log)


def memoryMaximum(log):
    return getStatFromLog('Max Memory', log)


@lru_cache()
def nEventsPerFile(sample):
    parentFile = get_test_file(sample)
    return get_eventN(parentFile)


def nTotalFiles(sample):
    return get_total_infiles(sample)


def nTotalEvents(sample):
    return nEventsPerFile(sample)*nTotalFiles(sample)


def getLogContents(skim, sample):
    try:
        logFileName = Path('log', f'{skim}_{sample}.out')
        with open(logFileName) as logFile:
            logContents = logFile.read()

        jsonFileName = Path('log', f'JobInformation_{skim}_{sample}.json')
        with open(jsonFileName) as jsonFile:
            jsonContents = json.load(jsonFile)
    except FileNotFoundError:
        raise SkimNotRunError(f'    Failed to find output files for {skim} skim on {sample} sample.\n' +
                              '    Perhaps you forgot to run the skim with runSkims.py?')

    return logContents, jsonContents


def testLogContents(logContents, jsonContents, skim, sample):
    logTests = ['Successfully completed' in logContents]
    jsonTests = [jsonContents['basf2_status']['finished'],
                 jsonContents['basf2_status']['success'],
                 jsonContents['basf2_status']['errors'] == 0,
                 jsonContents['basf2_status']['fatals'] == 0,
                 all(check for check in jsonContents['output_files'][0]['checks_passed'].values())
                 ]

    if not all(logTests) and all(jsonTests):
        raise SkimNotRunError(f'    Error found in log files of {skim} skim on {sample} sample.\n' +
                              '    Please check the .out, .err, and .json files in log/ directory.')


def getSkimsToRun():
    parser = argparse.ArgumentParser(description='A script to print tables of statistics for skims ' +
                                     'which have been run by runSkims.py. One or more standalone or combined ' +
                                     'skim names must be provided.',
                                     epilog='example: ./printSkimStats.py -s LeptonicUntagged -c BtoCharm')
    parser.add_argument('-s', '--standalone', nargs='+', default=[],
                        choices=['all']+allStandaloneSkims, metavar='SKIM',
                        help='List of standalone skims to run. Valid options are: ' + ', '.join(allStandaloneSkims) +
                        ', or all to run all standalone skims.')
    parser.add_argument('-c', '--combined', nargs='+', default=[],
                        choices=['all']+allCombinedSkims, metavar='SKIM',
                        help='List of combined skims to run. Valid options are: ' + ', '.join(allCombinedSkims) +
                        ', or all to run all combined skims.')
    args = parser.parse_args()

    if not (args.standalone or args.combined):
        parser.print_help()
        sys.exit(1)

    if args.standalone == ['all']:
        standaloneSkims = allStandaloneSkims
    else:
        standaloneSkims = args.standalone

    if args.combined == ['all']:
        combinedSkims = allCombinedSkims
    else:
        combinedSkims = args.combined

    return standaloneSkims + combinedSkims


def getSkimStatsDict(skims, samples, statistics):
    allSkimStats = {skim: {stat: {} for stat in statistics.keys()} for skim in skims}

    for skim in skims:
        try:
            for sample in samples:
                logContents, jsonContents = getLogContents(skim, sample)

                testLogContents(logContents, jsonContents, skim, sample)

                for statName, statInfo in statistics.items():
                    statFunction = statInfo['Calculate']

                    try:
                        allSkimStats[skim][statName][sample] = statFunction(jsonContents, logContents, sample)
                    except TypeError:
                        allSkimStats[skim][statName][sample] = None
        except SkimNotRunError as e:
            del allSkimStats[skim]

            print(f'Error! Could not get stats for {skim}. Details:', file=sys.stderr)
            print(e, '\n', file=sys.stderr)

    return allSkimStats


def mcWeightedAverage(statsPerSample):
    totalCrossSection = sum(mcSampleCrossSections.values())

    weightedAverage = 0
    for mcSample, crossSection in mcSampleCrossSections.items():
        for beamBackground, beamBackgroundWeight in beamBackgroundWeights.items():
            sample = f'{mcCampaign}_{mcSample}{beamBackground}'
            weightedAverage += statsPerSample[sample]*beamBackgroundWeight*crossSection/totalCrossSection

    return weightedAverage


def addWeightedMC(allSkimStats, statistics):
    for skimStats in allSkimStats.values():
        for statName, statInfo in statistics.items():
            try:
                combiningFunction = statInfo['Combine']
                skimStats[statName]['Combined MC'] = combiningFunction(skimStats[statName])
            except TypeError:
                skimStats[statName]['Combined MC'] = None

    return allSkimStats


def toJson(allSkimStats):
    outputJsonName = 'skimStats.json'
    with open(outputJsonName, 'w') as outputJson:
        json.dump(allSkimStats, outputJson, indent=4)
    print(f'Wrote stats to JSON file {outputJsonName}.')


def toScreen(allSkimStats):
    for skimName, skimStats in allSkimStats.items():
        print(f'Performance statistics for {skimName} skim:')

        # Only print some stats to screen
        selectedStats = [stat for (stat, statInfo) in statistics.items() if statInfo['PrintToScreen']]
        df = pd.DataFrame(skimStats, columns=selectedStats)

        df = df.reindex([*dataSamples.keys(), 'Combined MC', *mcSamples.keys()])
        df = df.rename(index={**dataSamples, **mcSamples})

        headers = ['\n'.join(wrap(statistics[stat]['LongName'], 12)) for stat in selectedStats]
        floatFormat = [''] + [statistics[stat]['FloatFormat'] for stat in selectedStats]

        table = tabulate(df[selectedStats],
                         headers=headers, tablefmt="fancy_grid",
                         numalign='right', stralign='left',
                         floatfmt=floatFormat)
        table = table.replace(' nan ', ' -   ')
        print(table)


def toConfluence(allSkimStats):
    confluenceStrings = []
    for skimName, skimStats in allSkimStats.items():
        confluenceStrings += [f'h1. Performance statistics for {skimName} skim']

        selectedStats = [stat for (stat, statInfo) in statistics.items() if statInfo['PrintToConfluence']]
        df = pd.DataFrame(skimStats, columns=selectedStats)

        # Set up row ordering and naming
        df = df.reindex([*dataSamples.keys(), 'Combined MC', *mcSamples.keys()])
        df = df.rename(index={**dataSamples, **mcSamples})

        headers = [statistics[stat]['LongName'] for stat in statistics]
        floatFormat = [''] + [statistics[stat]['FloatFormat'] for stat in statistics]

        table = tabulate(df, headers=headers, tablefmt="jira", floatfmt=floatFormat)

        # Make the first column (the sample label) bold on Confluence
        table = re.sub(r'^\| ', '|| ', table, flags=re.MULTILINE)
        table = table.replace(' nan ', ' --  ')

        confluenceStrings += [table]

    confluenceString = '\n'.join(confluenceStrings)

    confluenceFileName = 'skimStats_confluence.txt'
    with open(confluenceFileName, 'w') as confluenceFile:
        confluenceFile.write(confluenceString)
    print(f'Wrote tables to {confluenceFileName}. The contents of this file can be copied directly to Confluence.')

statistics = {
    'RetentionRate': {
        'LongName': 'Retention rate (%)',
        'FloatFormat': '.2f',
        'PrintToScreen': True,
        'PrintToConfluence': True,
        'Calculate': lambda json, *_: 100 * nSkimmedEvents(json) / nInputEvents(json),
        'Combine': lambda statDict: mcWeightedAverage(statDict)
    },
    'nInputEvents': {
        'LongName': 'Number of input events of test',
        'FloatFormat': '.0f',
        'PrintToScreen': False,
        'PrintToConfluence': True,
        'Calculate': lambda json, *_: nInputEvents(json),
        'Combine': None
    },
    'nSkimmedEvents': {
        'LongName': 'Number of skimmed events',
        'FloatFormat': '.0f',
        'PrintToScreen': False,
        'PrintToConfluence': True,
        'Calculate': lambda json, *_: nSkimmedEvents(json),
        'Combine': None
    },
    'cpuTime': {
        'LongName': 'CPU time of test on KEKCC (s)',
        'FloatFormat': '.1f',
        'PrintToScreen': False,
        'PrintToConfluence': True,
        'Calculate': lambda _, log, __: cpuTime(log),
        'Combine': None
    },
    'cpuTimePerEvent': {
        'LongName': 'CPU time per event on KEKCC (s)',
        'FloatFormat': '.3f',
        'PrintToScreen': True,
        'PrintToConfluence': True,
        'Calculate': lambda json, log, *_: cpuTime(log) / nInputEvents(json),
        'Combine': lambda statDict: mcWeightedAverage(statDict)
    },
    'udstSize': {
        'LongName': 'uDST size of test (MB)',
        'FloatFormat': '.2f',
        'PrintToScreen': False,
        'PrintToConfluence': True,
        'Calculate': lambda json, *_: udstSize(json) / 1024,
        'Combine': None
    },
    'udstSizePerEvent': {
        'LongName': 'uDST size per event (kB)',
        'FloatFormat': '.3f',
        'PrintToScreen': True,
        'PrintToConfluence': True,
        'Calculate': lambda json, *_: udstSize(json) / nInputEvents(json),
        'Combine': lambda statDict: mcWeightedAverage(statDict)
    },
    'udstSizePerFile': {
        'LongName': 'Estimated average uDST size per file (MB)',
        'FloatFormat': '.2f',
        'PrintToScreen': False,
        'PrintToConfluence': False,
        'Calculate': lambda json, _, sample: udstSize(json) / nInputEvents(json) * get_events_per_file(sample) / 1024,
        'Combine': lambda statDict: mcWeightedAverage(statDict)
    },
    'logSize': {
        'LongName': 'Log size of test (kB)',
        'FloatFormat': '.1f',
        'PrintToScreen': False,
        'PrintToConfluence': True,
        'Calculate': lambda _, log, __: logSize(log),
        'Combine': None
    },
    'logSizePerEvent': {
        'LongName': 'Log size per event (B)',
        'FloatFormat': '.2f',
        'PrintToScreen': True,
        'PrintToConfluence': True,
        'Calculate': lambda json, log, __: logSize(log) / nInputEvents(json) * 1024,
        'Combine': lambda statDict: mcWeightedAverage(statDict)
    },
    'averageCandidateMultiplicity': {
        'LongName': 'Average candidate multiplicity of passed events',
        'FloatFormat': '.2f',
        'PrintToScreen': True,
        'PrintToConfluence': True,
        'Calculate': lambda _, log, __: averageCandidateMultiplicity(log),
        'Combine': lambda statDict: mcWeightedAverage(statDict)
    },
    'memoryAverage': {
        'LongName': 'Average memory usage (MB)',
        'FloatFormat': '.0f',
        'PrintToScreen': True,
        'PrintToConfluence': True,
        'Calculate': lambda _, log, __: memoryAverage(log),
        'Combine': lambda statDict: mcWeightedAverage(statDict)
    },
    'memoryMaximum': {
        'LongName': 'Maximum memory usage (MB)',
        'FloatFormat': '.0f',
        'PrintToScreen': True,
        'PrintToConfluence': True,
        'Calculate': lambda _, log, __: memoryMaximum(log),
        'Combine': lambda statDict: max(statDict.values())
    },
    'udstSizePerEntireSample': {
        'LongName': 'Estimated uDST size for entire sample (GB)',
        'FloatFormat': '.2f',
        'PrintToScreen': False,
        'PrintToConfluence': True,
        'Calculate': lambda json, _, sample: udstSize(json) * nTotalEvents(sample) / nInputEvents(json) / 1024 / 1024,
        'Combine': lambda statDict: sum(statDict.values())
    },
    'logSizePerEntireSample': {
        'LongName': 'Estimated log size for entire sample (GB)',
        'FloatFormat': '.2f',
        'PrintToScreen': False,
        'PrintToConfluence': True,
        'Calculate': lambda json, log, sample: logSize(log) * nTotalEvents(sample) / nInputEvents(json) / 1024 / 1024,
        'Combine': lambda statDict: sum(statDict.values())
    }
}

if __name__ == '__main__':
    skims = getSkimsToRun()

    allSkimStats = getSkimStatsDict(skims, samples, statistics)

    allSkimStats = addWeightedMC(allSkimStats, statistics)

    toScreen(allSkimStats)
    toJson(allSkimStats)
    toConfluence(allSkimStats)
