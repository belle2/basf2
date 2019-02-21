#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
from modularAnalysis import inputMdst
from modularAnalysis import fillParticleList
from modularAnalysis import variablesToNtuple
from variables.utils import add_collection
# This import statement is put here for the use of lists of MCGenTopo variables
from variables.parameterCollections import MCGenTopo
# This import statement is put here for the use of collections of MCGenTopo variables
from variables import parameterCollections

# create path
my_path = basf2.create_path()

# load input ROOT file
inputMdst('default', basf2.find_file('JPsi2ee_e2egamma.root', 'examples', False), path=my_path)

# create and fill ParticleList
fillParticleList('e+', 'nTracks>=2 and abs(d0)<2 and abs(z0)<4 and clusterE>2', path=my_path)

# define variables
varE = ['M', 'px', 'py', 'pz', 'E']
add_collection(varE, 'varE')
varEvt = ['nTracks']
add_collection(varEvt, 'varEvt')

# output variables to ntuples with variablesToNtuple

# with a particle list, regular variables, using list of variables
variablesToNtuple('e+', varE, 'varE_1', 'test_detailed.root', path=my_path)
# with a particle list, regular variables, using collection of variables
variablesToNtuple('e+', ['varE'], 'varE_2', 'test_detailed.root', path=my_path)
# with a particle list, MCGenTopo variables with default parameter value 200, using list of variables
variablesToNtuple('e+', MCGenTopo(), 'MCGenTopoE_1', 'test_detailed.root', path=my_path)
# with a particle list, MCGenTopo variables with default parameter value 200, using collection of variables
variablesToNtuple('e+', ['MCGenTopo'], 'MCGenTopoE_2', 'test_detailed.root', path=my_path)
# with a particle list, MCGenTopo variables with specified parameter value 120, using list of variables
variablesToNtuple('e+', MCGenTopo(120), 'MCGenTopoE_120_1', 'test_detailed.root', path=my_path)
# with a particle list, MCGenTopo variables with specified parameter value 120, using collection of variables
variablesToNtuple('e+', ['MCGenTopo_120'], 'MCGenTopoE_120_2', 'test_detailed.root', path=my_path)
# with a particle list, regular variables + MCGenTopo variables with default parameter value 200, using list of variables
variablesToNtuple('e+', varE + MCGenTopo(), 'E_1', 'test_detailed.root', path=my_path)
# with a particle list, regular variables + MCGenTopo variables with default parameter value 200, using collection of variables
variablesToNtuple('e+', ['varE', 'MCGenTopo'], 'E_2', 'test_detailed.root', path=my_path)
# with a particle list, regular variables + MCGenTopo variables with specified parameter value 120, using list of variables
variablesToNtuple('e+', varE + MCGenTopo(120), 'E_120_1', 'test_detailed.root', path=my_path)
# with a particle list, regular variables + MCGenTopo variables with specified parameter value 120, using collection of variables
variablesToNtuple('e+', ['varE', 'MCGenTopo_120'], 'E_120_2', 'test_detailed.root', path=my_path)

# with no particle list, regular variables, using list of variables
variablesToNtuple('', varEvt, 'varEvt_1', 'test_detailed.root', path=my_path)
# with no particle list, regular variables, using collection of variables
variablesToNtuple('', ['varEvt'], 'varEvt_2', 'test_detailed.root', path=my_path)
# with no particle list, MCGenTopo variables with default parameter value 200, using list of variables
variablesToNtuple('', MCGenTopo(), 'MCGenTopoEvt_1', 'test_detailed.root', path=my_path)
# with no particle list, MCGenTopo variables with default parameter value 200, using collection of variables
variablesToNtuple('', ['MCGenTopo'], 'MCGenTopoEvt_2', 'test_detailed.root', path=my_path)
# with no particle list, MCGenTopo variables with specified parameter value 120, using list of variables
variablesToNtuple('', MCGenTopo(120), 'MCGenTopoEvt_120_1', 'test_detailed.root', path=my_path)
# with no particle list, MCGenTopo variables with specified parameter value 120, using collection of variables
variablesToNtuple('', ['MCGenTopo_120'], 'MCGenTopoEvt_120_2', 'test_detailed.root', path=my_path)
# with no particle list, regular variables + MCGenTopo variables with default parameter value 200, using list of variables
variablesToNtuple('', varEvt + MCGenTopo(), 'Evt_1', 'test_detailed.root', path=my_path)
# with no particle list, regular variables + MCGenTopo variables with default parameter value 200, using collection of variables
variablesToNtuple('', ['varEvt', 'MCGenTopo'], 'Evt_2', 'test_detailed.root', path=my_path)
# with no particle list, regular variables + MCGenTopo variables with specified parameter value 120, using list of variables
variablesToNtuple('', varEvt + MCGenTopo(120), 'Evt_120_1', 'test_detailed.root', path=my_path)
# with no particle list, regular variables + MCGenTopo variables with specified parameter value 120, using collection of variables
variablesToNtuple('', ['varEvt', 'MCGenTopo_120'], 'Evt_120_2', 'test_detailed.root', path=my_path)

# Process the events
basf2.process(my_path)

# print out the summary
print(basf2.statistics)
