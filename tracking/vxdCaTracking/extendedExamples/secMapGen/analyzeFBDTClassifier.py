#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *
from sys import argv

from VXDTF.setup_modules_ml import add_fbdtclassifier_analyzer

# default values
fbdtName = 'FBDTClassifier_training.dat'
trainSamp = 'FBDTClassifier_samples.dat'
testSamp = 'FBDtClassifier_samples_test.dat'
outputFN = 'FBDTClassifier_analyze.root'

# read in values
if len(argv) > 1:
    fbdtName = str(argv[1])
if len(argv) > 2:
    trainSamp = str(argv[2])
if len(argv) > 3:
    testSamp = str(argv[3])
if len(argv) > 4:
    outputFN = str(argv[4])

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [0])
eventinfosetter.param('evtNumList', [0])  # no need for events only need initialize and terminate

main = create_path()
main.add_module(eventinfosetter)

add_fbdtclassifier_analyzer(main, fbdtName, trainSamp, testSamp, outputFN)

process(main)
