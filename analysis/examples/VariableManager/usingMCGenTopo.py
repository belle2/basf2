#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# This example demonstrates the detailed usage of the parameter lists of variables returned by mc_gen_topo(n).
# For details on mc_gen_topo(n), please refer to its definition in analysis/scripts/variables/MCGenTopo.py.

import basf2
from modularAnalysis import inputMdst
from modularAnalysis import fillParticleList
from modularAnalysis import variablesToNtuple
from variables.utils import add_collection
# This import statement is put here for the use of lists of mc_gen_topo variables
from variables.MCGenTopo import mc_gen_topo

# create path
my_path = basf2.create_path()

# load input ROOT file
inputMdst(basf2.find_file('JPsi2ee_e2egamma.root', 'examples', False), path=my_path)

# create and fill ParticleList
fillParticleList('e+', 'nTracks>=2 and abs(d0)<2 and abs(z0)<4 and clusterE>2', path=my_path)

# define variables
var_e = ['M', 'px', 'py', 'pz', 'E']
add_collection(var_e, 'var_e')
var_evt = ['nTracks']
add_collection(var_evt, 'var_evt')
# Collections associated with mc_gen_topo(n) are not defined by default.
# If you want to use them, just define them with add_collection as the two instances above.
# Here are the two instances that will be used in the following part of the script.
add_collection(mc_gen_topo(), 'mc_gen_topo')
add_collection(mc_gen_topo(120), 'mc_gen_topo_120')

# output variables to ntuples with variablesToNtuple

# with a particle list, regular variables, using list of variables
variablesToNtuple('e+', var_e, 'var_e_1', 'MCGenTopoVariables.root', path=my_path)
# with a particle list, regular variables, using collection of variables
variablesToNtuple('e+', ['var_e'], 'var_e_2', 'MCGenTopoVariables.root', path=my_path)
# with a particle list, mc_gen_topo variables with default parameter value 200, using list of variables
variablesToNtuple('e+', mc_gen_topo(), 'mc_gen_topo_e_1', 'MCGenTopoVariables.root', path=my_path)
# with a particle list, mc_gen_topo variables with default parameter value 200, using collection of variables
variablesToNtuple('e+', ['mc_gen_topo'], 'mc_gen_topo_e_2', 'MCGenTopoVariables.root', path=my_path)
# with a particle list, mc_gen_topo variables with specified parameter value 120, using list of variables
variablesToNtuple('e+', mc_gen_topo(120), 'mc_gen_topo_e_120_1', 'MCGenTopoVariables.root', path=my_path)
# with a particle list, mc_gen_topo variables with specified parameter value 120, using collection of variables
variablesToNtuple('e+', ['mc_gen_topo_120'], 'mc_gen_topo_e_120_2', 'MCGenTopoVariables.root', path=my_path)
# with a particle list, regular variables + mc_gen_topo variables with default parameter value 200, using list of variables
variablesToNtuple('e+', var_e + mc_gen_topo(), 'e_1', 'MCGenTopoVariables.root', path=my_path)
# with a particle list, regular variables + mc_gen_topo variables with default parameter value 200, using collection of variables
variablesToNtuple('e+', ['var_e', 'mc_gen_topo'], 'e_2', 'MCGenTopoVariables.root', path=my_path)
# with a particle list, regular variables + mc_gen_topo variables with specified parameter value 120, using list of variables
variablesToNtuple('e+', var_e + mc_gen_topo(120), 'e_120_1', 'MCGenTopoVariables.root', path=my_path)
# with a particle list, regular variables + mc_gen_topo variables with specified parameter value 120, using collection of variables
variablesToNtuple('e+', ['var_e', 'mc_gen_topo_120'], 'e_120_2', 'MCGenTopoVariables.root', path=my_path)

# with no particle list, regular variables, using list of variables
variablesToNtuple('', var_evt, 'var_evt_1', 'MCGenTopoVariables.root', path=my_path)
# with no particle list, regular variables, using collection of variables
variablesToNtuple('', ['var_evt'], 'var_evt_2', 'MCGenTopoVariables.root', path=my_path)
# with no particle list, mc_gen_topo variables with default parameter value 200, using list of variables
variablesToNtuple('', mc_gen_topo(), 'mc_gen_topo_evt_1', 'MCGenTopoVariables.root', path=my_path)
# with no particle list, mc_gen_topo variables with default parameter value 200, using collection of variables
variablesToNtuple('', ['mc_gen_topo'], 'mc_gen_topo_evt_2', 'MCGenTopoVariables.root', path=my_path)
# with no particle list, mc_gen_topo variables with specified parameter value 120, using list of variables
variablesToNtuple('', mc_gen_topo(120), 'mc_gen_topo_evt_120_1', 'MCGenTopoVariables.root', path=my_path)
# with no particle list, mc_gen_topo variables with specified parameter value 120, using collection of variables
variablesToNtuple('', ['mc_gen_topo_120'], 'mc_gen_topo_evt_120_2', 'MCGenTopoVariables.root', path=my_path)
# with no particle list, regular variables + mc_gen_topo variables with default parameter value 200, using list of variables
variablesToNtuple('', var_evt + mc_gen_topo(), 'evt_1', 'MCGenTopoVariables.root', path=my_path)
# with no particle list, regular variables + mc_gen_topo variables with default parameter value 200, using collection of variables
variablesToNtuple('', ['var_evt', 'mc_gen_topo'], 'evt_2', 'MCGenTopoVariables.root', path=my_path)
# with no particle list, regular variables + mc_gen_topo variables with specified parameter value 120, using list of variables
variablesToNtuple('', var_evt + mc_gen_topo(120), 'evt_120_1', 'MCGenTopoVariables.root', path=my_path)
# with no particle list, regular variables + mc_gen_topo variables with specified parameter value 120, using collection of variables
variablesToNtuple('', ['var_evt', 'mc_gen_topo_120'], 'evt_120_2', 'MCGenTopoVariables.root', path=my_path)

# Process the events
basf2.process(my_path)

# print out the summary
print(basf2.statistics)
