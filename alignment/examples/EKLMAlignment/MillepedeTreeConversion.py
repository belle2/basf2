#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from basf2 import *
import ROOT
from ROOT import Belle2
import numpy as np

main = create_path()

algo = Belle2.MillepedeTreeConversionAlgorithm()
algo.setInputFileNames([sys.argv[1]])
algo.setOutputFile('millepede_data.root')
algo.execute()
