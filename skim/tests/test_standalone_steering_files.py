#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
This script runs each of the standalone steering files in the skim package with a small number of
events, and tests whether the steering files run successfully.
"""

__author__ = 'Phil Grace'
__email__ = 'philip.grace@adelaide.edu.au'

import re
import subprocess
import sys

from b2test_utils import clean_working_directory, require_file
from basf2 import find_file
from skim.registry import Registry, combined_skims


def getSkimStandaloneScript(skimName, combined=False):
    """Finds the standalone skim script for the given skim name.

    Args:
        skimName (str): The name of the skim for which to find the standalone script.
        combined (bool): Pass `True` to get the skim script for a combined skim.

    Returns:
        skimScript (str): The absolute path to the standalone script in the currently set up basf2
            installation.
    """
    if combined:
        scriptDirectory = 'combined'
    else:
        scriptDirectory = 'standalone'

    try:
        skimScript = find_file(f'skim/{scriptDirectory}/{skimName}_Skim_Standalone.py')
        return skimScript
    except FileNotFoundError:
        print(f'WARNING! Skim {skimName} registered in skim registry, but no standalone steering file found!', file=sys.stderr)
        return None


def filterErrorMessages(output):
    """Filter the output of basf2 to only print WARNING, ERROR, and FATAL messages.

    Args:
        output (str): Output to be filtered.
    Returns:
        filteredOutput (str): Output filtered to only problematic lines.
    """
    matchingLines = re.findall(r'.WARNING.*|.ERROR.*|.FATAL.*', output)
    return '\n'.join(matchingLines)


def testSteeringFile(skimScript):
    """Runs the standalone script for a given skim. If it fails, STDERR and the error messages from
    basf2 are printed to the screen.

    Args:
        skimScript (str): The path to the standalone skim script to be tested.
    Returns:
        steeringFileWorks (bool): True if basf2 exited normally, and False otherwise.
    """
    testFile = require_file('mdst12.root', 'validation')

    with clean_working_directory():
        process = subprocess.run(['basf2', skimScript, '-n', '1', '-i', testFile],
                                 universal_newlines=True,
                                 stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    steeringFileWorks = (process.returncode == 0)

    if steeringFileWorks:
        print(f'Script {skimScript} ran successfully.')
    else:
        print(f'ERROR! Script {skimScript} failed! See output below.', file=sys.stderr)

        filteredStdout = filterErrorMessages(process.stdout)
        print(process.stderr, filteredStdout, file=sys.stderr)

    return steeringFileWorks


def testAllSkims(standaloneSkims, combinedSkims):
    """Tests steering files for given lists of skims, and prints a report about which failed to run.
    If any failed, exits with return code 1.

    Args:
        standaloneSkims (list): List of names of individual skims to test.
        combinedSkims (list): List of names of combined skims to test.
    """
    standaloneScripts = [getSkimStandaloneScript(skim) for skim in standaloneSkims]
    combinedScripts = [getSkimStandaloneScript(skim, combined=True) for skim in combinedSkims]

    allScripts = filter(None, standaloneScripts + combinedScripts)

    failedScripts = [script for script in allScripts if not testSteeringFile(script)]

    if failedScripts:
        print('The following standalone steering files failed:', file=sys.stderr)
        print('\n'.join(failedScripts), file=sys.stderr)

        sys.exit(1)


if __name__ == '__main__':
    standaloneSkims = Registry.names
    combinedSkims = combined_skims.keys()

    testAllSkims(standaloneSkims, combinedSkims)
