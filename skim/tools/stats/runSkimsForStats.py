#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
"""

import argparse
from pathlib import Path
import re
import subprocess
import sys

from ROOT import PyConfig

# Importing ROOT in skimExpertFunctions has the side-effect of hijacking argument parsing
PyConfig.IgnoreCommandLineOptions = True
from b2test_utils import clean_working_directory
from basf2 import find_file
from skim.registry import skim_registry, combined_skims
from skimExpertFunctions import get_test_file


allStandaloneSkims = [skim for _, skim in skim_registry]
allCombinedSkims = list(combined_skims.keys())

mcCampaign = 'MC12'
beamBackgrounds = ['BGx1', 'BGx0']
mcSampleTypes = ['mixed', 'charged', 'ccbar', 'uubar', 'ddbar', 'ssbar', 'taupair']

mcSamples = [f'{mcCampaign}_{mcSample}{beamBackground}'
             for beamBackground in beamBackgrounds
             for mcSample in mcSampleTypes]

dataSamples = ['proc9_exp3', 'proc9_exp7', 'proc9_exp8', 'bucket7_exp8']

samples = mcSamples + dataSamples

nTestEvents = 10000


def getArgumentParser():
    """

    Returns:
        parser (argparse.ArgumentParser):
    """
    allStandaloneSkims = [skim for _, skim in skim_registry]
    allCombinedSkims = list(combined_skims.keys())

    parser = argparse.ArgumentParser(description='A script to run a given set of skims, and ' +
                                     'save the output in a format to be read by printSkimStats.py. ' +
                                     'One or more standalone or combined skim names must be provided.',
                                     epilog='example: ./printSkimStats.py -s LeptonicUntagged -c BtoCharm')
    parser.add_argument('-s', '--standalone', nargs='+', default=[],
                        choices=['all']+allStandaloneSkims, metavar='SKIM',
                        help='List of standalone skims to run. Valid options are: ' +
                        ', '.join([f'``{s}``' for s in allStandaloneSkims]) +
                        ', or ``all`` to run all standalone skims.')
    parser.add_argument('-c', '--combined', nargs='+', default=[],
                        choices=['all']+allCombinedSkims, metavar='SKIM',
                        help='List of combined skims to run. Valid options are: ' +
                        ', '.join([f'``{s}``' for s in allCombinedSkims]) +
                        ', or ``all`` to run all combined skims.')

    return parser


def getSkimsAndScriptsToRun(parser, standaloneList, combinedList):
    """
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

    standaloneScripts = [find_file(f'skim/standalone/{skim}_Skim_Standalone.py', silent=True) for skim in standaloneSkims]
    combinedScripts = [find_file(f'skim/combined/{skim}_Skim_Standalone.py', silent=True) for skim in combinedSkims]

    skims = standaloneSkims + combinedSkims
    scripts = standaloneScripts + combinedScripts

    return skims, scripts

if __name__ == '__main__':
    parser = getArgumentParser()
    args = parser.parse_args()
    skims, scripts = getSkimsAndScriptsToRun(parser, args.standalone, args.combined)

    logDirectory = Path('log').resolve()
    logDirectory.mkdir(parents=True, exist_ok=True)

    for skim, script in zip(skims, scripts):
        if not script:
            print(f'Error! Could not find script for {skim} skim.', file=sys.stderr)
            continue

        jobIDs = []
        returnCodes = []

        for sample in samples:
            sampleFile = get_test_file(sample)

            logFile = Path(logDirectory, f'{skim}_{sample}.out')
            errFile = Path(logDirectory, f'{skim}_{sample}.err')
            jsonFile = Path(logDirectory, f'JobInformation_{skim}_{sample}.json')
            print(f'{skim}, {sample}')
            with clean_working_directory():
                process = subprocess.run(['bsub', '-q', 'l', '-oo', logFile, '-e', errFile,
                                          '-J', f'{skim} {sample}'
                                          'basf2', script, '--job-information', jsonFile,
                                          '-n', str(nTestEvents), '-i', sampleFile],
                                         stdout=subprocess.PIPE, stderr=subprocess.PIPE)

            jobIDs.append(re.findall('\d+', str(process.stdout))[0])
            returnCodes.append(process.returncode)

        if any(returnCodes):
            print(f'An error occurred while submitting jobs for {skim} skim with script {script}.')
        else:
            print(f'Running {script} on {nTestEvents} events for each test sample. Job IDs:\n  ' + '\n  '.join(jobIDs))
