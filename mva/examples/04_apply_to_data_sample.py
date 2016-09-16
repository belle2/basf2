#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Thomas Keck 2016

# Use training in a basf2 path
# Run basics.py before

from basf2 import *
from modularAnalysis import *
import create_data_sample

if __name__ == "__main__":
    path = create_data_sample.reconstruction_path(['/storage/jbod/tkeck/MC6/charged/sub00/mdst_0001*.root'])
    path.add_module('MVAExpert', listNames=['D0'], extraInfoName='Test', identifier='weightfile.root')
    path.add_module('MVAExpert', listNames=['D0'], extraInfoName='Pdf', identifier='MVAPdf')
    path.add_module('MVAExpert', listNames=['D0'], extraInfoName='Full', identifier='MVAFull')
    path.add_module('MVAExpert', listNames=['D0'], extraInfoName='Ordinary', identifier='MVAOrdinary')
    path.add_module('MVAExpert', listNames=['D0'], extraInfoName='SPlot', identifier='MVASPlot')
    path.add_module('MVAExpert', listNames=['D0'], extraInfoName='SPlotCombined', identifier='MVASPlotCombined')
    path.add_module('MVAExpert', listNames=['D0'], extraInfoName='SPlotBoosted', identifier='MVASPlotBoosted')
    path.add_module('MVAExpert', listNames=['D0'], extraInfoName='SPlotCombinedBoosted', identifier='MVASPlotCombinedBoosted')
    variablesToNTuple('D0', ['isSignal', 'extraInfo(Pdf)', 'extraInfo(Full)', 'extraInfo(Ordinary)', 'extraInfo(SPlot)',
                             'extraInfo(SPlotCombined)', 'extraInfo(SPlotBoosted)', 'extraInfo(SPlotCombinedBoosted)'], path=path)
    process(path)
