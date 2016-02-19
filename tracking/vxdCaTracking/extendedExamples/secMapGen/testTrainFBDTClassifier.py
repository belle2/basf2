#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *
from sys import argv

from VXDTF.setup_modules_ml import *

# default values
bdtOutName = 'FBDTClassifier_training.dat'
samplesName = 'FBDTClassifier_samples.dat'
nTrees = 100
treeD = 3
shrink = 0.15
ratio = 0.5

# read in values
if len(argv) > 1:
    samplesName = str(argv[1])
if len(argv) > 2:
    bdtOutName = str(argv[2])
if len(argv) > 3:
    nTrees = int(argv[3])
if len(argv) > 4:
    treeD = int(argv[4])
if len(argv) > 5:
    shrink = float(argv[5])
if len(argv) > 6:
    ratio = float(argv[6])


eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [0])
eventinfosetter.param('evtNumList', [0])  # no need for events only need initialize and terminate

main = create_path()
main.add_module(eventinfosetter)

add_fbdtclassifier_training(main, '', bdtOutName, True, False, True, samplesName, nTrees, treeD, shrink, ratio, LogLevel.DEBUG, 100)

process(main)
