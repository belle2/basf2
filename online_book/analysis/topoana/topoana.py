##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2
from modularAnalysis import inputMdst
from modularAnalysis import variablesToNtuple
from variables.MCGenTopo import mc_gen_topo
import os						# Newly added statement 1!

mypath = basf2.Path()

# load input ROOT file
inputMdst('default', basf2.find_file('B02D0pi0_D02pi0pi0.root', 'examples', False), mypath)

# Output the variables to a ntuple
variablesToNtuple('', mc_gen_topo(200), 'MCGenTopo', 'MCGenTopo.root', path=mypath)

# Process the events
basf2.process(mypath)

# Print out the summary
print(basf2.statistics)

# Invoke the TopoAna program
os.system('topoana.exe topoana.card')			# Newly added statement 2!
