#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *
import ROOT
from ROOT import Belle2

set_debug_level(1000)
use_local_database('localdb/database.txt')

# Only initialize RootInput, as we do not loop over events,
# only load persistent objects stored during data collection
input = register_module('RootInput')
input.param('inputFileName', 'RootOutput.root')
input.initialize()

components = ROOT.vector('string')()
components.push_back('CDCAlignment')

algo = Belle2.MillepedeAlgorithm()
algo.setComponents(components)
algo.steering().command('method diagonalization 3 0.1')
algo.steering().command('entries 10')
algo.steering().command('hugecut 50')
algo.steering().command('threads 4 4')
algo.steering().command('chiscut 30. 6.')
algo.steering().command('outlierdownweighting 3')
algo.steering().command('dwfractioncut 0.1')

algo.steering().command('Parameters')


def fixLayerParam(layer, param):
    label = Belle2.GlobalLabel()
    label.construct(Belle2.CDCAlignment.getGlobalUniqueID(), layer, param)
    algo.steering().command(str(label.label()) + ' 0.0 -1.')

fixLayerParam(0, 1)  # X
fixLayerParam(0, 2)  # Y
fixLayerParam(55, 1)  # X
fixLayerParam(55, 2)  # Y

"""
algo.steering().command('fortranfiles')
algo.steering().command('constraints.txt')
"""

algo.invertSign()
algo.execute()


# Skip into interactive environment
# Exit with Ctrl+D
print('++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++')
print(' You are now in interactive environment. You can still access the algorithm.')
print(' Please review the output and if fine, type: ')
print('')
print(' algo.commit()')
print('')
print(' Press Ctrl+[D] and [Enter] to exit.')
print('++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++')

import interactive
interactive.embed()
