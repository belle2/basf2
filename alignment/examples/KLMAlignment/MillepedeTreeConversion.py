#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import basf2 as b2
from ROOT import Belle2

main = b2.create_path()

algo = Belle2.MillepedeTreeConversionAlgorithm()
algo.setInputFileNames([sys.argv[1]])
algo.setOutputFile('millepede_data.root')
algo.execute()
