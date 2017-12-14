#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *
import ROOT
from ROOT import Belle2

set_debug_level(1000)
use_local_database('localdb/database.txt')

components = ROOT.vector('string')()
components.push_back('CDCAlignment')
components.push_back('CDCLayerAlignment')

algo = Belle2.MillepedeAlgorithm()
algo.setInputFileNames(['CollectorOutput.root'])

algo.setComponents(components)
algo.invertSign()
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
    label.construct(Belle2.CDCLayerAlignment.getGlobalUniqueID(), layer, param)
    algo.steering().command(str(label.label()) + ' 0.0 -1.')


fixLayerParam(0, 1)  # X
fixLayerParam(0, 2)  # Y
fixLayerParam(55, 1)  # X
fixLayerParam(55, 2)  # Y

"""
for layer in range(0, 56):
    if layer == 2:
      continue

    fixLayerParam(layer, 6)  # Y


for layer in range(0, 56):
    fixLayerParam(layer, 1)  # Y
    fixLayerParam(layer, 2)  # Y
"""
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
