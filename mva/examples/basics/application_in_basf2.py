#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Thomas Keck 2016

# Use training in a basf2 path
# Run basics/*.py before

from basf2 import *
from modularAnalysis import *
import create_data_sample

if __name__ == "__main__":
    path = create_data_sample.reconstruction_path(['mdst_002001_prod00000789_task00004203.root'])
    path.add_module('MVAExpert', listNames=['D0'], extraInfoName='Test', identifier='weightfile.root')
    variablesToNtuple('D0', ['isSignal', 'extraInfo(Test)'], path=path)
    process(path)
