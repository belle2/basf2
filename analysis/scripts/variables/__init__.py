#!/usr/bin/env python3

import argparse
import basf2.utils as b2utils
import ROOT
from ROOT import Belle2
"""
Makes analysis variables (which can be used in many of the functions defined
in :mod:`modularAnalysis`) available to Python.
Details can be found in the VariableManager section at https://software.belle2.org/
"""
from ROOT import gSystem
gSystem.Load('libanalysis.so')

#: import everything into current namespace.
variables = Belle2.Variable.Manager.Instance()


def std_vector(*args):
    """
    Creates an std::vector which can be passed to pyROOT
    """
    v = ROOT.std.vector(type(args[0]))()
    for x in args:
        v.push_back(x)
    return v


def getCommandLineOptions():
    """ Parses the command line options and returns the corresponding arguments. """
    parser = argparse.ArgumentParser()
    parser.add_argument('--no-pager', dest='pager', default=True, action='store_false',
                        help='Use a pager to show output or print to terminal.')
    args = parser.parse_args()
    return args


def printVars(changedVariableNames=None):
    """
    Print list of all available variables.
    """

    if changedVariableNames:
        print(changedVariableNames)
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
    b2utils.pretty_print_description_list(rows)


def getAllTrgNames():
    """
    Return all PSNM trigger bit names
    """

    bits = Belle2.PyDBObj('TRGGDLDBFTDLBits')
    evt = Belle2.EventMetaData()
    Belle2.DBStore.Instance().update(evt)
    size = Belle2.TRGSummary.c_ntrgWords * Belle2.TRGSummary.c_trgWordSize
    return [bits.getoutbitname(i) for i in range(size) if bits.getoutbitname(i) != '']
