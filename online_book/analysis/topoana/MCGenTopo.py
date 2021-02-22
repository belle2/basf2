import basf2
from modularAnalysis import inputMdst
from modularAnalysis import variablesToNtuple
from variables.MCGenTopo import mc_gen_topo

mypath = basf2.Path()

# load input ROOT file
inputMdst('default', '/group/belle2/dataprod/MC/SkimTraining/mixed_BGx1.mdst_000001_prod00009434_task10020000001.root', path=mypath)

# Output the variables to a ntuple
variablesToNtuple('', mc_gen_topo(200), 'MCGenTopo', 'MCGenTopo.root', path=mypath)

# Process the events
basf2.process(mypath)

# print out the summary
print(basf2.statistics)
