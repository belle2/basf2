#! /usr/bin/env python3
import subprocess

from hlt.clean_execution import CleanBasf2Execution
from argparse import ArgumentParser


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('script_name', type=str,
                        help='Which script to execute')

    args, remaining = parser.parse_known_args()
    execution = CleanBasf2Execution(["basf2", "--no-stats", args.script_name, "--"] + remaining)
    try:
        execution.run()
    finally:
        subprocess.run("framework-pcore-clear_ipcs")
