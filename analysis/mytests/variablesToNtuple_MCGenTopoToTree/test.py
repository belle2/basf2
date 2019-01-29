#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
from modularAnalysis import inputMdst
from modularAnalysis import fillParticleList
from modularAnalysis import variablesToNtuple
from modularAnalysis import MCGenTopoToTree
from modularAnalysis import variablesToNtuple_MCGenTopoToTree

# create path
my_path = basf2.create_path()

# load input ROOT file
inputMdst('default', basf2.find_file('JPsi2ee_e2egamma.root', 'examples', False), path=my_path)

# create and fill ParticleList
fillParticleList('e+', 'nTracks>=2 and abs(d0)<2 and abs(z0)<4 and clusterE>2', path=my_path)

# define variables
varE = ['M', 'px', 'py', 'pz', 'E']
varEvt = ['nMCParticles']

# output variables to ntuples with variablesToNtuple, and output MCGenTopo to trees with MCGenTopoToTree
variablesToNtuple('e+', varE, 'varE', 'test1.root', path=my_path)
variablesToNtuple('', varEvt, 'varEvt', 'test1.root', path=my_path)
MCGenTopoToTree('e+', 'MCGenTopoE', 'test1.root', path=my_path)
MCGenTopoToTree('', 'MCGenTopoEvt', 'test1.root', path=my_path)

# output variables to ntuples and meanwhile write MCGenTopo to trees with variablesToNtuple_MCGenTopoToTree
variablesToNtuple_MCGenTopoToTree('e+', varE, 'varE', 'test2.root', path=my_path)
variablesToNtuple_MCGenTopoToTree('', varEvt, 'varEvt', 'test2.root', path=my_path)

# Process the events
basf2.process(my_path)

# print out the summary
print(basf2.statistics)
