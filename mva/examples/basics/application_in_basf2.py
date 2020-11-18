#!/usr/bin/env python3
# Thomas Keck 2016

# Use training in a basf2 path
# Run basics/*.py before

import basf2 as b2
import modularAnalysis as ma
import create_data_sample

if __name__ == "__main__":
    path = create_data_sample.reconstruction_path(['mdst_002001_prod00000789_task00004203.root'])
    path.add_module('MVAExpert', listNames=['D0'], extraInfoName='Test', identifier='weightfile.root')
    ma.variablesToNtuple('D0', ['isSignal', 'extraInfo(Test)'], path=path)
    b2.process(path)
