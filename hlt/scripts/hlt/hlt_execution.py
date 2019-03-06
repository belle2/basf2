#! /usr/bin/env python3
import os
import subprocess
import sys

from hlt.clean_execution import CleanBasf2Execution


def main(script_name):
    """Shortcut method to do the basf2 execution on expressreco and hlt"""
    os.chdir(sys.path[0])
    execution = CleanBasf2Execution()
    try:
        execution.start(["basf2", "--no-stats", script_name, "--"] + sys.argv[1:])
        return_code = execution.wait()
    finally:
        subprocess.run("framework-pcore-clear_ipcs")
    return return_code
