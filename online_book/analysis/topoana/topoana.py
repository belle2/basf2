import basf2
from modularAnalysis import inputMdst
from modularAnalysis import variablesToNtuple
from variables.MCGenTopo import mc_gen_topo
import os

mypath = basf2.Path()

# load input ROOT file
inputMdst('default', basf2.find_file('B02D0pi0_D02pi0pi0.root', 'examples', False), mypath)

# Output the variables to a ntuple
variablesToNtuple('', mc_gen_topo(200), 'MCGenTopo', 'MCGenTopo.root', path=mypath)

# Process the events
basf2.process(mypath)

# print out the summary
print(basf2.statistics)

# Invoke the TopoAna program
os.system('topoana.exe topoana.card')
