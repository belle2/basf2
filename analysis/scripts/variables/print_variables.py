#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
You can run ``basf2 variables.py`` to list all available variables.
"""

import basf2.utils as b2utils
import pager
import argparse
from variables import variables as cpp_variables


def getCommandLineOptions():
    """ Parses the command line options of the fei and returns the corresponding arguments. """
    parser = argparse.ArgumentParser()
    parser.add_argument('--no-pager', dest='pager', default=True, action='store_false',
                        help='Use a pager to show output or print to terminal.')
    args = parser.parse_args()
    return args


def printVars():
    """
    Print list of all available variables.
    """

    print('Available variables in Variable::Manager:')
    allVars = cpp_variables.getVariables()
    vars = []
    for v in allVars:
        vars.append((v.group, v.name, v.description))

    rows = []
    current_group = ''
    for (group, name, description) in sorted(vars):
        if current_group != group:
            current_group = group
            rows.append([group])
        rows.append([name, description])
    b2utils.pretty_print_description_list(rows)


if __name__ == "__main__":
    args = getCommandLineOptions()

    if args.pager:
        with pager.Pager(r'Available variables in Variable\:\:Manager'):
            printVars()
    else:
        printVars()
