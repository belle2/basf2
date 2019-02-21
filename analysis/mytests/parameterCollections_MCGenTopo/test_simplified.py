#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
from modularAnalysis import inputMdst
from modularAnalysis import fillParticleList
from modularAnalysis import variablesToNtuple
from variables.parameterCollections import MCGenTopo

# create path
my_path = basf2.create_path()

# load input ROOT file
inputMdst('default', basf2.find_file('JPsi2ee_e2egamma.root', 'examples', False), path=my_path)

# create and fill ParticleList
fillParticleList('e+', 'nTracks>=2 and abs(d0)<2 and abs(z0)<4 and clusterE>2', path=my_path)

# define variables
var = ['M', 'px', 'py', 'pz', 'E']

# output regular variables to ntuple 'var' with variablesToNtuple
variablesToNtuple('e+', var, 'var', 'test_simplified.root', path=my_path)
# output MCGenTopo variables to ntuple 'MCGenTopo' with variablesToNtuple
variablesToNtuple('e+', MCGenTopo(), 'MCGenTopo', 'test_simplified.root', path=my_path)
# output both regular and MCGenTopo variables to ntuple 'var_MCGenTopo' with variablesToNtuple
variablesToNtuple('e+', var + MCGenTopo(), 'var_MCGenTopo', 'test_simplified.root', path=my_path)

# Process the events
basf2.process(my_path)

# print out the summary
print(basf2.statistics)
