#!/usr/bin/env python3
# Thomas Keck 2016

# Use training in a basf2 path
# Run basics/*.py before

import basf2 as b2
import modularAnalysis as ma
import create_data_sample

if __name__ == "__main__":
    path = create_data_sample.reconstruction_path([b2.find_file('Dst2D0pi.root', 'examples', False)])
    path.add_module('MVAExpert', listNames=['D0'], extraInfoName='Test', identifier='weightfile.root')
    ma.variablesToNtuple('D0', ['isSignal', 'extraInfo(Test)'], path=path)
    b2.process(path)
