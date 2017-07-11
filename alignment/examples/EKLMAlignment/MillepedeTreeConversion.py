#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from basf2 import *
import ROOT
from ROOT import Belle2
import numpy as np

main = create_path()

input = register_module('RootInput')
input.param('inputFileName', sys.argv[1])
input.initialize()

algo = Belle2.MillepedeTreeConversionAlgorithm()
algo.setOutputFile('millepede_data.root')
algo.execute()
