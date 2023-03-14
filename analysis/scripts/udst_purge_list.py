#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# @cond

import basf2
from mdst import add_mdst_output
import modularAnalysis as ma
import vertex as vtx
from variables import variables as vm
import argparse


def preprocess_roe_data(listName, charge, path):
    ''' Process/select events on top of the skim processing:
    -- vertex fit
    -- select of the positive/negative charge
    -- extra cuts
    '''
    # charge
    if not (charge == 0):
        if listName.find("B0") == 0:
            basf2.B2ERROR(f"Inconsistency between neutral list {listName} and charge requirement {charge}")
        ma.variablesToEventExtraInfo(listName, {'charge': 'bcharge'}, path=path)
        vm.addAlias('bcharge', 'eventExtraInfo(bcharge)')

        # Filter:
        empty_path = basf2.create_path()
        cfilter = basf2.register_module('VariableToReturnValue')
        cfilter.param('variable', 'bcharge')
        cfilter.if_value(f'!={charge}', empty_path)
        path.add_module(cfilter).set_name("chargeFilter")

    # vertex fit
    vtx.treeFit(listName, path=path)
    ma.variablesToEventExtraInfo(listName, {'chiProb': 'chiProb'}, path=path)
    vm.addAlias('fitProb', 'eventExtraInfo(chiProb)')

    # filter failed fits ...
    empty_path2 = basf2.create_path()
    fitfilter = basf2.register_module('VariableToReturnValue')
    fitfilter.param('variable', 'fitProb')
    fitfilter.if_value('<0', empty_path2)
    path.add_module(fitfilter).set_name("fitFilter")

    # require one candidate per event
    vm.addAlias('nB', f'nParticlesInList({listName})')
    eventFilter = basf2.register_module('VariableToReturnValue')
    eventFilter.param('variable', 'nB')
    eventFilter.if_value('!=1', empty_path2)
    path.add_module(eventFilter).set_name("eventFilter")

    # store extra momentum/vertex position information
    Vars = {'px': 'PX', 'py': 'PY', 'pz': 'PZ', 'x': 'X', 'y': 'Y', 'z': 'Z',
            'IPX': 'IPX', 'IPY': 'IPY', 'IPZ': 'IPZ'}
    ma.variablesToEventExtraInfo(listName, Vars, path=path)


def list_select_roe_data(listName, path):

    filter1 = basf2.register_module("UdstListFilter")
    filter1.param("listName", listName)
    filter1.param("keepNotInList", True)
    path.add_module(filter1).set_name("filter1")


def preprocess_signal_mc(listName, charge, path):
    # charge
    if not (charge == 0):
        if listName.find("B0") == 0:
            basf2.B2ERROR(f"Inconsistency between neutral list {listName} and charge requirement {charge}")
        ma.variablesToEventExtraInfo(listName, {'charge': 'bcharge'}, path=path)
        vm.addAlias('bcharge', 'eventExtraInfo(bcharge)')
        empty_path = basf2.create_path()
        cfilter = basf2.register_module('VariableToReturnValue')
        cfilter.param('variable', 'bcharge')
        cfilter.if_value(f'!={charge}', empty_path)
        path.add_module(cfilter).set_name("chargeFilter")

    vm.addAlias('nB', f'nParticlesInList({listName})')
    empty_path2 = basf2.create_path()
    eventFilter = basf2.register_module('VariableToReturnValue')
    eventFilter.param('variable', 'nB')
    eventFilter.if_value('!=1', empty_path2)
    path.add_module(eventFilter).set_name("eventFilter")

    Vars = {'mcPX': 'PX', 'mcPY': 'PY', 'mcPZ': 'PZ',
            'mcDecayVertexX': 'X',
            'mcDecayVertexY': 'Y',
            'mcDecayVertexZ': 'Z',
            'IPX': 'IPX', 'IPY': 'IPY', 'IPZ': 'IPZ'
            }
    ma.variablesToEventExtraInfo(listName, Vars, path=path)


def list_select_signal_mc(listName, path):
    filter2 = basf2.register_module("UdstListFilter")
    filter2.param("listName", listName)
    filter2.param("keepNotInList", False)
    path.add_module(filter2).set_name("filter2")


def prepare_path(FileIn, ListName, icharge, isROE, FileOut):
    '''
    Return processing path
       FileIn   -- input file name
       ListName -- B+ or B0 decay chain of the skim
       icharge  -- charge (0, -1 or +1)
       isROE    -- ROE (data) if true, signal (MC) if false
       FileOut  -- output file name
    '''
    main = basf2.Path()

    # input
    input1 = basf2.register_module('RootInput')
    input1.param('inputFileName', FileIn)
    main.add_module(input1).set_name("input1")

    if (isROE):
        preprocess_roe_data(ListName, icharge, main)
        list_select_roe_data(ListName, main)
    else:
        preprocess_signal_mc(ListName, icharge, main)
        list_select_signal_mc(ListName, main)

    # output
    additionalBranches = ['EventExtraInfo']

    add_mdst_output(main, filename=FileOut, additionalBranches=additionalBranches)

    # progress
    main.add_module('Progress')

    basf2.print_path(main)
    return main


def get_parser():
    """Handles the command-line argument parsing.

    Returns:
        argparse.Namespace: The parsed arguments.
    """
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("--fileIn",  help='Input file name.', required=True)
    parser.add_argument('--listName', help='Name of the particle list to keep (isSignal) or to remove.', required=True)
    parser.add_argument("--isSignal", action='store_true', help='If added, the input is signal MC')
    parser.add_argument("--charge", default='pos',
                        help='Filter charge. Possible values are :code:`pos`, :code:`neg` or :code:`zero`. Default is :code:`pos`')
    parser.add_argument("--fileOut", default='test.root', help='Output file name. Default: :code:`test.root`')

    return parser


############################################################################################################
if __name__ == '__main__':

    args = get_parser().parse_args()

    list1 = args.listName
    signal = args.isSignal
    charge = args.charge
    fileOut = args.fileOut
    fileIn = args.fileIn

    if charge == 'zero':
        icharge = 0
    elif charge == 'pos':
        icharge = 1
    elif charge == 'neg':
        icharge = -1

    main = prepare_path(fileIn, list1, icharge, not signal, fileOut)
    basf2.process(main)

    print(basf2.statistics)
