#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# EKLM alingment along strips.

from basf2 import *
from reconstruction import *
from ROOT import Belle2
import sys

# Set the global log level
set_log_level(LogLevel.INFO)

gearbox = register_module('Gearbox')
gearbox.initialize()

algo = Belle2.EKLMAlignmentAlongStripsAlgorithm()
algo.setInputFileNames(sys.argv[1:])
algo.setOutputFile('alignment_along_strips.root')
algo.execute()
algo.commit()
