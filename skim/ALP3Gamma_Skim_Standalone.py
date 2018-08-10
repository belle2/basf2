from basf2 import *
from modularAnalysis import *  # Standard Analysis Tools
from stdPhotons import *
from skimExpertFunctions import encodeSkimName
skimCode = encodeSkimName('ALP3Gamma')

# fileList contains the input file which the skim will run on
fileList = []

inputMdstList('MC9', fileList)

# importing the reconstructed events from the ALP_List file
from skim.dark import ALP3GammaList
BeamList = ALP3GammaList()

# output to Udst file
skimOutputUdst(skimCode, BeamList)

# print out Particle List statistics
summaryOfLists(BeamList)

process(analysis_main)

# print out the summary
print(statistics)
