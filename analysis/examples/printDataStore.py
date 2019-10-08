#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This script loads specified ROOT file and prints the content of the DataStore
for each event. To be used for debugging.

Execute script with:
  $> basf2 printDataStore.py -i [input_ROOT_file]

"""

import basf2
from modularAnalysis import printDataStore

path = basf2.Path()
path.add_module('RootInput')
printDataStore(path=path)
basf2.process(path)
