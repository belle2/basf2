#! /usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import subprocess
import sys

from hlt.clean_execution import CleanBasf2Execution


def main(script_name):
    """Shortcut method to do the basf2 execution on expressreco and hlt"""
    execution = CleanBasf2Execution(timeout=10)
    try:
        execution.start(["basf2", "--no-stats", script_name, "--"] + sys.argv[1:])
        return_code = execution.wait()
    finally:
        subprocess.run("framework-pcore-clear_ipcs")
    return return_code
