from basf2 import *
from modularAnalysis import *  # Standard Analysis Tools
from stdPhotons import *

# fileList contains the input file which the skim will run on
fileList = []

inputMdstList('default', fileList)

# fillParticleList('gamma:tight', '', True)

# importing the reconstructed events from the ALP_List file
from ALP_List import *
BeamList = beam()

for k in BeamList:
    print('ciao')
    print(k)
    print()

# output to Udst file
skimOutputUdst('ALP_', BeamList)

# Print out Particle List statistics
summaryOfLists(BeamList)

process(analysis_main)

# print out the summary
print(statistics)
