#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# EKLM alingment along strips.

import basf2 as b2
from ROOT import Belle2
import sys

# Set the global log level
b2.set_log_level(b2.LogLevel.INFO)

gearbox = b2.register_module('Gearbox')
gearbox.initialize()

algo = Belle2.EKLMAlignmentAlongStripsAlgorithm()
algo.setInputFileNames(sys.argv[1:])
algo.setOutputFile('alignment_along_strips.root')
algo.execute()
algo.commit()
