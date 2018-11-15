from basf2 import *
from modularAnalysis import *  # Standard Analysis Tools
from stdPhotons import *
from skimExpertFunctions import encodeSkimName
skimCode = encodeSkimName('ALP3Gamma')

# fileList contains the input file which the skim will run on

fileList = \
    [
        '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
        'mdst_000001_prod00002288_task00000001.root'
    ]
inputMdstList('MC9', fileList)

from skim.dark import ALP3GammaList
BeamList = ALP3GammaList()

# output to Udst file
skimOutputUdst(skimCode, BeamList)

# print out Particle List statistics
summaryOfLists(BeamList)

process(analysis_main)

# print out the summary
print(statistics)
