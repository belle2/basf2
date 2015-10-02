#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import gSystem
gSystem.Load('libanalysis.so')

# import everything into current namespace.
from ROOT import Belle2
variables = Belle2.Variable.Manager.Instance()


def printVars():
    """
    Print list of all available variables.
    """

    print('Available variables in Variable::Manager:')
    allVars = variables.getVariables()
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
    pretty_print_description_list(rows)


if __name__ == "__main__":
    printVars()
