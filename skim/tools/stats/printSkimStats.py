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


def getAllSamples(mcCampaign):
    mcSampleLabels = {
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

    dataSampleLabels = {
        'proc9_exp3': 'Data: proc9 exp. 3',
        'proc9_exp7': 'Data: proc9 exp. 7',
        'proc9_exp8': 'Data: proc9 exp. 8',
        'bucket7_exp8': 'Data: bucket7 exp. 8'
    }

    return mcSampleLabels, dataSampleLabels


class SkimNotRunException(Exception):
    """An exception to be raised whenever an error occurs that is likely due to
    a skim not being run properly by ``submitTestSkims.py``.
    """
    pass


class CustomHelpFormatter(argparse.HelpFormatter):
    """Custom formatter for argparse, to print the valid choices for an argument
    in the help string.
    """
    def _get_help_string(self, action):
        if action.choices:
            return action.help + ' Valid options are: ' + ', '.join(action.choices)
        else:
            return action.help


def getArgumentParser():
    """

    Returns:
        parser (argparse.ArgumentParser):
    """
    allStandaloneSkims = [skim for _, skim in skim_registry]
    allCombinedSkims = list(combined_skims.keys())

    parser = argparse.ArgumentParser(description='A script to print tables of statistics for skims ' +
                                     'which have been run by ``submitTestSkims.py``. One or more standalone or combined ' +
                                     'skim names must be provided.',
                                     formatter_class=CustomHelpFormatter)
    parser.add_argument('-s', '--standalone', nargs='+', default=[],
                        choices=['all']+allStandaloneSkims, metavar='SKIM',
                        help='List of standalone skims to run.')
    parser.add_argument('-c', '--combined', nargs='+', default=[],
                        choices=['all']+allCombinedSkims, metavar='SKIM',
                        help='List of combined skims to run.')
    parser.add_argument('--mccampaign', default='MC12',
                        choices=['MC9', 'MC10', 'MC11', 'MC12'],
                        help='The MC campaign to test on.')

    sampleGroup = parser.add_mutually_exclusive_group()
    sampleGroup.add_argument('--mconly', action='store_true',
                             help='Test on only MC samples.')
    sampleGroup.add_argument('--dataonly', action='store_true',
                             help='Test on only data samples.')

    return parser


def getStatFromLog(statisticName, logFileContents):
    """Search for a given statistic in the "Resource usage summary" section of the log file.

    Args:
        statisticName (str): The name of the value as it appears in the log file.
        logFileContents (str): A string containing the contents of the log file
            of a skim script.

    Returns:
        statFromLog (float):
    """
    floatRegexp = r'\s*:\s+(\d+(\.(\d+)?)?)'
    statFromLog = re.findall(f'{statisticName}{floatRegexp}', logFileContents)[0][0]

    return float(statFromLog)


def nInputEvents(jsonFileContents):
    """

    Args:
        jsonFileContents (dict): A dict read from the JobInformation JSON output
            of a skim script.

    Returns:
        nInputEvents (int):
    """
    return jsonFileContents['basf2_status']['total_events']


def nSkimmedEvents(jsonFileContents):
    """

    Args:
        jsonFileContents (dict): A dict read from the JobInformation JSON output
            of a skim script.

    Returns:
        nSkimmedEvents (int): The average number of events in the output uDST files.
    """
    outputFileInfoList = jsonFileContents['output_files']
    eventNumberList = [outputFileInfo['stats']['events'] for outputFileInfo in outputFileInfoList]

    return sum(eventNumberList)/len(eventNumberList)


def udstSize(jsonFileContents):
    """Return the size of the output uDST file in KB, read from Job Information JSON file.

    Args:
        jsonFileContents (dict): A dict read from the JobInformation JSON output
            of a skim script.

    Returns:
        udstSize (float): The average file size of the output uDST files.
    """
    outputFileInfoList = jsonFileContents['output_files']
    udstSizeList = [outputFileInfo['stats']['filesize_kib'] for outputFileInfo in outputFileInfoList]

    return sum(udstSizeList)/len(udstSizeList)


def logSize(logFileContents):
    """Return the size of the log file in kB.

    Args:
        logFileContents (str): A string containing the contents of the log file
            of a skim script.

    Returns:
        logSize (float):
    """
    return len(logFileContents) / 1024


def cpuTime(logFileContents):
    """

    Args:
        logFileContents (str): A string containing the contents of the log file
            of a skim script.

    Returns:
        cpuTime (float):
    """
    return getStatFromLog('CPU time', logFileContents)


def averageCandidateMultiplicity(logFileContents):
    """

    Args:
        logFileContents (str): A string containing the contents of the log file
            of a skim script.

    Returns:
        averageCandidateMultiplicity (float):
    """
    # Find all the blocks with the header "Average Candidate Multiplicity", and
    # join them into a single string.
    multiplicityBlocks = re.findall('(Average Candidate Multiplicity(.*\n)+?.*INFO)', logFileContents)
    joinedBlocks = '\n'.join([block[0] for block in multiplicityBlocks])

    # Get the second numbers in each line, which are the average candidate
    # multiplicities of passed events for each particle list.
    floatRegexp = r'\d+\.\d+'
    multiplicityLines = [line for line in joinedBlocks.split('\n') if re.findall(floatRegexp, line)]
    multiplicities = [float(re.findall(floatRegexp, line)[1]) for line in multiplicityLines]

    averageCandidateMultiplicity = sum(multiplicities)/len(multiplicities)

    return averageCandidateMultiplicity


def memoryAverage(logFileContents):
    """

    Args:
        logFileContents (str): A string containing the contents of the log file
            of a skim script.

    Returns:
       memoryAverage (float):
    """
    return getStatFromLog('Average Memory', logFileContents)


def memoryMaximum(logFileContents):
    """

    Args:
        logFileContents (str): A string containing the contents of the log file
            of a skim script.

    Returns:
        memoryMaximum (float):
    """
    return getStatFromLog('Max Memory', logFileContents)


@lru_cache()
def nEventsPerFile(sample):
    """

    Args:
        sample (str): The label of the sample being tested.

    Returns:
        nEventsPerFile (int):
    """
    parentFile = get_test_file(sample)
    return get_eventN(parentFile)


def nTotalFiles(sample):
    """

    Args:
        sample (str): The label of the sample being tested.

    Returns:
        nTotalFiles (int):
    """
    return get_total_infiles(sample)


def nTotalEvents(sample):
    """

    Args:
        sample (str): The label of the sample being tested.

    Returns:
        nTotalEvents (int):
    """
    return nEventsPerFile(sample)*nTotalFiles(sample)


def getJobOutput(skim, sample):
    """

    Args:
        skim (str): The name of the skim being tested.
        sample (str): The label of the sample being tested.

    Returns:
        logFileContents (str): The contents of the log file.
        jsonFileContents (dict): A dict read from the JobInformation JSON output
            of a skim script.

    Raises:
        SkimNotRunException: Raised if any of the log or JSON files cannot be opened.
    """
    try:
        logFileName = Path('log', f'{skim}_{sample}.out')
        with open(logFileName) as logFile:
            logFileContents = logFile.read()

        jsonFileName = Path('log', f'JobInformation_{skim}_{sample}.json')
        with open(jsonFileName) as jsonFile:
            jsonFileContents = json.load(jsonFile)
    except FileNotFoundError:
        raise SkimNotRunException(f'    Failed to open output files for {skim} skim on {sample} sample.\n' +
                                  '    Perhaps you forgot to run the skim with runSkimsForStats.py?')

    return logFileContents, jsonFileContents


def testLogContents(logFileContents, jsonFileContents, skim, sample):
    """

    Args:
        logFileContents (str):  A string containing the contents of the log file
            of a skim script.
        jsonFileContents (dict): A dict read from the JobInformation JSON output
            of a skim script.
        skim (str): The name of the skim being tested.
        sample (str): The label of the sample being tested.

    Raises:
        SkimNotRunException: Raised if the log file indicates the job did not finish
            successfully, or the JSON file indicates the output files were not
            written correctly.
    """
    logTests = ['Successfully completed' in logFileContents]

    outputFileInfoList = jsonFileContents['output_files']

    jsonTests = [jsonFileContents['basf2_status']['finished'],
                 jsonFileContents['basf2_status']['success'],
                 jsonFileContents['basf2_status']['errors'] == 0,
                 jsonFileContents['basf2_status']['fatals'] == 0,
                 all(check for outputFileInfo in outputFileInfoList for check in outputFileInfo['checks_passed'].values())
                 ]

    if not all(logTests) and all(jsonTests):
        raise SkimNotRunException(f'    Error found in log files of {skim} skim on {sample} sample.\n' +
                                  '    Please check the .out, .err, and .json files in log/ directory.')


def getSkimsToRun(parser, standaloneList, combinedList):
    """

    Returns:
        skimsToRun (list):
    """
    if not (standaloneList or combinedList):
        parser.print_help()
        sys.exit(1)

    if standaloneList == ['all']:
        standaloneSkims = allStandaloneSkims
    else:
        standaloneSkims = standaloneList

    if combinedList == ['all']:
        combinedSkims = allCombinedSkims
    else:
        combinedSkims = combinedList

    return standaloneSkims + combinedSkims


def getSamplesToRun(mcSamples, dataSamples, mcOnly=False, dataOnly=False):
    """
    """
    if mcOnly:
        return mcSamples
    elif dataOnly:
        return dataSamples
    else:
        return mcSamples + dataSamples


def getSkimStatsDict(skims, samples, statSpecifier):
    """
    If ``SkimNotRunException`` is raised, this is caught and the dict entries for
    the skim are removed. An error messsage is printed, and execution continues.

    Args:
        skims (list): List of skims to run.
        samples (list): List of sample labels to test on.
        statSpecifier (dict): A nested dict specifying how each statistic should
            be calculated and printed, as returned by `getStatSpecifier`.

    Returns:
        skimStatsDict (dict): A nested dict indexed as ``skim name::statistic
            label::sample label``.
    """
    allSkimStats = {skim: {stat: {} for stat in statSpecifier.keys()} for skim in skims}

    for skim in skims:
        try:
            for sample in samples:
                logFileContents, jsonFileContents = getJobOutput(skim, sample)

                testLogContents(logFileContents, jsonFileContents, skim, sample)

                for statName, statInfo in statSpecifier.items():
                    statFunction = statInfo['Calculate']

                    try:
                        allSkimStats[skim][statName][sample] = statFunction(jsonFileContents, logFileContents, sample)
                    except TypeError:
                        allSkimStats[skim][statName][sample] = None
        except SkimNotRunException as e:
            del allSkimStats[skim]

            print(f'Error! Could not get stats for {skim}. Details:', file=sys.stderr)
            print(e, '\n', file=sys.stderr)

    return allSkimStats


def mcWeightedAverage(statsPerSample):
    """

    Args:
        statsPerSample (dict):

    Returns:
        mcWeightedAverage (float):
    """

    # The fraction of each background level produced in the MC sample
    beamBackgroundFractions = {
        'BGx1': 0.8,
        'BGx0': 0.2
    }

    # The cross section (nb) of each process in e+e- collisions
    processCrossSections = {
        'mixed': 0.555,
        'charged': 0.555,
        'ccbar': 1.3,
        'uubar': 1.61,
        'ddbar': 0.40,
        'ssbar': 0.38,
        'taupair': 0.91
    }

    totalCrossSection = sum(processCrossSections.values())

    weightedAverage = 0
    for process, crossSection in processCrossSections.items():
        for beamBackground, beamBackgroundFraction in beamBackgroundFractions.items():
            sample = f'{mcCampaign}_{process}{beamBackground}'
            weightedAverage += statsPerSample[sample]*beamBackgroundFraction*crossSection/totalCrossSection

    return weightedAverage


def addWeightedMC(allSkimStats, statSpecifier, mcCampaign):
    """

    Args:
        allSkimStats (dict):
        statSpecifier (dict):

    Returns:
        allSkimStatsWithWeightedMC (dict):
    """
    for skimStats in allSkimStats.values():
        for statName, statInfo in statSpecifier.items():
            try:
                combiningFunction = statInfo['CombineMC']
                skimStats[statName]['Combined MC'] = combiningFunction(skimStats[statName], mcCampaign)
            except TypeError:
                skimStats[statName]['Combined MC'] = None

    return allSkimStats


def printToJson(allSkimStats, statSpecifier):
    """

    Args:
        allSkimStats (dict):
    """
    outputJsonName = 'skimStats.json'
    with open(outputJsonName, 'w') as outputJson:
        json.dump(allSkimStats, outputJson, indent=4)
    print(f'\nWrote stats to JSON file {outputJsonName}.')


def printToScreen(allSkimStats, statSpecifier, mcOnly, dataOnly):
    """

    Args:
        allSkimStats (dict):
    """
    selectedStats = [stat for (stat, statInfo) in statSpecifier.items() if statInfo['PrintToScreen']]

    headers = ['\n'.join(wrap(statSpecifier[stat]['LongName'], 12)) for stat in selectedStats]
    floatFormat = [''] + [statSpecifier[stat]['FloatFormat'] for stat in selectedStats]

    for skimName, skimStats in allSkimStats.items():
        print(f'\nPerformance statistics for {skimName} skim:')

        # Only print some stats to screen
        df = pd.DataFrame(skimStats, columns=selectedStats)

        # Handle skims only being run on data or MC
        if mcOnly:
            index = ['Combined MC', *mcSampleLabels.keys()]
            renamingPairs = mcSampleLabels
        elif dataOnly:
            index = list(dataSampleLabels.keys())
            renamingPairs = dataSampleLabels
        else:
            index = [*dataSampleLabels.keys(), 'Combined MC', *mcSampleLabels.keys()]
            renamingPairs = {**dataSampleLabels, **mcSampleLabels}

        df = df.reindex(index)
        df = df.rename(index=renamingPairs)

        table = tabulate(df[selectedStats],
                         headers=headers, tablefmt="fancy_grid",
                         numalign='right', stralign='left',
                         floatfmt=floatFormat)
        table = table.replace(' nan ', ' -   ')
        print(table)


def printToConfluence(allSkimStats, statSpecifier):
    """

    Args:
        allSkimStats (dict):
    """
    confluenceFileName = 'skimStats_confluence.txt'

    selectedStats = [stat for (stat, statInfo) in statSpecifier.items() if statInfo['PrintToConfluence']]

    headers = [statSpecifier[stat]['LongName'] for stat in selectedStats]
    floatFormat = [''] + [statSpecifier[stat]['FloatFormat'] for stat in selectedStats]

    confluenceStrings = ['h1. How each statistic is calculated']
    for stat in selectedStats:
        confluenceStrings.append(' - ' + statSpecifier[stat]['LongName'] + ': ' + statSpecifier[stat]['CalculationDescription'])

    for skimName, skimStats in allSkimStats.items():
        confluenceStrings += ['', f'h1. Performance statistics for {skimName} skim']

        df = pd.DataFrame(skimStats, columns=selectedStats)

        # Set up row ordering and naming
        df = df.reindex([*dataSampleLabels.keys(), 'Combined MC', *mcSampleLabels.keys()])
        df = df.rename(index={**dataSampleLabels, **mcSampleLabels})

        table = tabulate(df, headers=headers, tablefmt="jira", floatfmt=floatFormat)

        # Make the first column (the sample label) bold on Confluence
        table = re.sub(r'^\| ', '|| ', table, flags=re.MULTILINE)
        table = table.replace(' nan ', ' --  ')

        confluenceStrings += [table]

    confluenceString = '\n'.join(confluenceStrings)

    with open(confluenceFileName, 'w') as confluenceFile:
        confluenceFile.write(confluenceString)

    print(f'\nWrote tables to {confluenceFileName}. The contents of this file can ' +
          'be copied directly to Confluence as Wiki markup in the markup editor ' +
          '(accessible via ctrl-shift-D or cmd-shift-D).')


def getStatSpecifier():
    """Returns a nested dict containing the specifications for how each
    statistic should be calculated and printed. The keys of the dict are used
    internally by this script for constructing the tables of stats. The entries
    of the dicts are as follows:

    * ``'LongName'`` (`str`): The label to use in the Confluence table. Should
      include units of the statistic.

    * ``'FloatFormat'`` (`str`): A printf string for how the number should be
      printed.

    * ``'PrintToScreen'`` (`bool`): Determines whether the statistic is included
      in the table in terminal output.

    * ``'PrintToConfluence'`` (`bool`): Determines whether the statistic is
      included in the Confluence table.

    * ``'Calculate'`` (`function`): An anonymous function for how to calculate
      the statistic. The arguments of this function must be a dict constructed
      from the JobInformation JSON output, a string containing the log file
      contents, and the name of the sample being used. Not all of these
      arguments must be used by the function, but they must all be supplied.

    * ``'CalculationDescription'`` (`str`): A sentence description of how the
      statistic is calculated.

    * ``'CombineMC'`` (`function`): An anonymous function for how this
      statistics should be combined across samples to obtain an estimate for a
      cross-section weighted MC sample. Can be `None` if there is no sensible
      way to combine this statistic for different MC samples. Should take as
      arguments a dict of stats for a single skim and single statistic, indexed
      by sample name, and the MC campaign number.

    Returns:
        statSpecifier (dict): A nested dict specifying how each
        statistic should be handled.
    """
    statSpecifier = {
        'RetentionRate': {
            'LongName': 'Retention rate (%)',
            'FloatFormat': '.2f',
            'PrintToScreen': True,
            'PrintToConfluence': True,
            'Calculate': lambda json, *_: 100 * nSkimmedEvents(json) / nInputEvents(json),
            'CalculationDescription': 'Number of skimmed events divided by number of input events.',
            'CombineMC': lambda statDict, mcCampaign: mcWeightedAverage(statDict, mcCampaign)
        },
        'nInputEvents': {
            'LongName': 'Number of input events of test',
            'FloatFormat': '.0f',
            'PrintToScreen': False,
            'PrintToConfluence': True,
            'Calculate': lambda json, *_: nInputEvents(json),
            'CalculationDescription': 'Number of events in the input MDST file.',
            'CombineMC': None
        },
        'nSkimmedEvents': {
            'LongName': 'Number of skimmed events',
            'FloatFormat': '.0f',
            'PrintToScreen': False,
            'PrintToConfluence': True,
            'Calculate': lambda json, *_: nSkimmedEvents(json),
            'CalculationDescription': 'Number of events in the output uDST file.',
            'CombineMC': None
        },
        'cpuTime': {
            'LongName': 'CPU time of test on KEKCC (s)',
            'FloatFormat': '.1f',
            'PrintToScreen': False,
            'PrintToConfluence': True,
            'Calculate': lambda _, log, __: cpuTime(log),
            'CalculationDescription': 'CPU time of test job as printed in the output logs.',
            'CombineMC': None
        },
        'cpuTimePerEvent': {
            'LongName': 'CPU time per event on KEKCC (s)',
            'FloatFormat': '.3f',
            'PrintToScreen': True,
            'PrintToConfluence': True,
            'Calculate': lambda json, log, *_: cpuTime(log) / nInputEvents(json),
            'CalculationDescription': 'CPU time of test job divided by number of input events.',
            'CombineMC': lambda statDict, mcCampaign: mcWeightedAverage(statDict, mcCampaign)
        },
        'udstSize': {
            'LongName': 'uDST size of test (MB)',
            'FloatFormat': '.2f',
            'PrintToScreen': False,
            'PrintToConfluence': True,
            'Calculate': lambda json, *_: udstSize(json) / 1024,
            'CalculationDescription': 'File size of output uDST.',
            'CombineMC': None
        },
        'udstSizePerEvent': {
            'LongName': 'uDST size per event (kB)',
            'FloatFormat': '.3f',
            'PrintToScreen': True,
            'PrintToConfluence': True,
            'Calculate': lambda json, *_: udstSize(json) / nInputEvents(json),
            'CalculationDescription': 'File size of output uDST divided by number of input events.',
            'CombineMC': lambda statDict, mcCampaign: mcWeightedAverage(statDict, mcCampaign)
        },
        'udstSizePerFile': {
            'LongName': 'Estimated average uDST size per file (MB)',
            'FloatFormat': '.2f',
            'PrintToScreen': False,
            'PrintToConfluence': False,
            'Calculate': lambda json, _, sample: udstSize(json) / nInputEvents(json) * get_events_per_file(sample) / 1024,
            'CalculationDescription': 'uDST size per event multiplied by average number of events per file for this sample.',
            'CombineMC': lambda statDict, mcCampaign: mcWeightedAverage(statDict, mcCampaign)
        },
        'logSize': {
            'LongName': 'Log size of test (kB)',
            'FloatFormat': '.1f',
            'PrintToScreen': False,
            'PrintToConfluence': True,
            'Calculate': lambda _, log, __: logSize(log),
            'CalculationDescription': 'File size of the output logs.',
            'CombineMC': None
        },
        'logSizePerEvent': {
            'LongName': 'Log size per event (B)',
            'FloatFormat': '.2f',
            'PrintToScreen': True,
            'PrintToConfluence': True,
            'Calculate': lambda json, log, __: logSize(log) / nInputEvents(json) * 1024,
            'CalculationDescription': 'File size of the output logs divided by the number of input events.',
            'CombineMC': lambda statDict, mcCampaign: mcWeightedAverage(statDict, mcCampaign)
        },
        'averageCandidateMultiplicity': {
            'LongName': 'Average candidate multiplicity of passed events',
            'FloatFormat': '.2f',
            'PrintToScreen': True,
            'PrintToConfluence': True,
            'Calculate': lambda _, log, __: averageCandidateMultiplicity(log),
            'CalculationDescription': 'Candidate multiplicity as listed in output logs, averaged over the particle lists.',
            'CombineMC': lambda statDict, mcCampaign: mcWeightedAverage(statDict, mcCampaign)
        },
        'memoryAverage': {
            'LongName': 'Average memory usage (MB)',
            'FloatFormat': '.0f',
            'PrintToScreen': True,
            'PrintToConfluence': True,
            'Calculate': lambda _, log, __: memoryAverage(log),
            'CalculationDescription': 'Average memory usage, as listed in output logs.',
            'CombineMC': lambda statDict, mcCampaign: mcWeightedAverage(statDict, mcCampaign)
        },
        'memoryMaximum': {
            'LongName': 'Maximum memory usage (MB)',
            'FloatFormat': '.0f',
            'PrintToScreen': True,
            'PrintToConfluence': True,
            'Calculate': lambda _, log, __: memoryMaximum(log),
            'CalculationDescription': 'Maximum memory usage, as listed in output logs.',
            'CombineMC': lambda statDict, _: max(statDict.values())
        },
        'udstSizePerEntireSample': {
            'LongName': 'Estimated uDST size for entire sample (GB)',
            'FloatFormat': '.2f',
            'PrintToScreen': False,
            'PrintToConfluence': True,
            'Calculate': lambda json, _, sample: udstSize(json) * nTotalEvents(sample) / nInputEvents(json) / 1024 / 1024,
            'CalculationDescription': 'Output uDST size per event multiplied by total number of events in the full sample.',
            'CombineMC': lambda statDict, _: sum(statDict.values())
        },
        'logSizePerEntireSample': {
            'LongName': 'Estimated log size for entire sample (GB)',
            'FloatFormat': '.2f',
            'PrintToScreen': False,
            'PrintToConfluence': True,
            'Calculate': lambda json, log, sample: logSize(log) * nTotalEvents(sample) / nInputEvents(json) / 1024 / 1024,
            'CalculationDescription': 'Output log file size per event multiplied by total number of events in the full sample.',
            'CombineMC': lambda statDict, _: sum(statDict.values())
        }
    }

    return statSpecifier

if __name__ == '__main__':
    parser = getArgumentParser()
    args = parser.parse_args()

    skims = getSkimsToRun(parser, args.standalone, args.combined)
    mcSampleLabels, dataSampleLabels = getAllSamples(args.mccampaign)
    samples = getSamplesToRun(list(mcSampleLabels.keys()), list(dataSampleLabels.keys()), args.mconly, args.dataonly)

    statSpecifier = getStatSpecifier()
    allSkimStats = getSkimStatsDict(skims, samples, statSpecifier)

    if not args.dataonly:
        allSkimStats = addWeightedMC(allSkimStats, statSpecifier, args.mccampaign)

    printToScreen(allSkimStats, statSpecifier, args.mconly, args.dataonly)
    printToJson(allSkimStats, statSpecifier)
    printToConfluence(allSkimStats, statSpecifier)
