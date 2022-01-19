##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import argparse
import basf2.utils as b2utils


def get_variable_manager():
    """
    Simple wrapper for returning an instance to the variable manager object.
    This is necessary for avoiding to import ROOT globally.
    """
    import ROOT
    variable_manager = ROOT.Belle2.Variable.Manager.Instance()
    return variable_manager


#: import everything into current namespace.
variables = get_variable_manager()


def std_vector(*args):
    """
    Creates an std::vector which can be passed to pyROOT
    """
    import ROOT
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
    import ROOT
    bits = ROOT.Belle2.PyDBObj('TRGGDLDBFTDLBits')
    evt = ROOT.Belle2.EventMetaData()
    ROOT.Belle2.DBStore.Instance().update(evt)
    size = ROOT.Belle2.TRGSummary.c_ntrgWords * ROOT.Belle2.TRGSummary.c_trgWordSize
    return [bits.getoutbitname(i) for i in range(size) if bits.getoutbitname(i) != '']
