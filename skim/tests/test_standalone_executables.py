#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
This script runs each of the standalone executables in the skim package with a small number of
events, and tests whether the executable run successfully.
"""

__author__ = 'Phil Grace'
__email__ = 'philip.grace@adelaide.edu.au'

from os import getenv
from pathlib import Path
import subprocess
import sys
from termcolor import colored, cprint


def getSkimStandaloneScript(skimName, combined=False):
    """Finds the standalone skim script for the given skim name.

    Args:
        skimName (str): The name of the skim for which to find the standalone script.
        combined (bool): Pass `True` to get the skim script for a combined skim.

    Returns:
        skimScript (str): The absolute path to the standalone script in the currently set up basf2
            installation.
    """
    basf2Directory = getenv('BELLE2_RELEASE', getenv('BELLE2_LOCAL_DIR'))

    if combined:
        scriptDirectory = 'combined'
    else:
        scriptDirectory = 'standalone'

    skimScript = str(Path(basf2Directory, 'skim', scriptDirectory, f'{skimName}_Skim_Standalone.py'))

    return skimScript


def testExecutable(skimName):
    """Runs the standalone script for a given skim. If it fails, the STDOUT of the script is printed
    to the screen. Otherwise, it is suppressed.

    Args:
        skimName (str): The name of the skim to be tested.
    Returns:
        executableWorks (bool): True if skim standalone script exited with exit code 0, and False otherwise.
    """
    skimScript = getSkimStandaloneScript(skimName)
    print(f'Testing script {skimScript}...')

    process = subprocess.run(['basf2', skimScript, '-n', '10'], universal_newlines=True,
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    executableWorks = (process.returncode == 0)

    if not executableWorks:
        cprint(f'Error: Script {skimScript} failed! See output below.', 'red')
        print(subprocess.stdout)
    else:
        cprint(f'Script {skimScript} worked.', 'green')

    return executableWorks


def main():
    executableWorks = testExecutable('SLUntagged')

if __name__ == '__main__':
    main()
