#######################################################
#
# Charmless 3-body skims
# C-L. Hsu, 03/Jun/2016
#
######################################################
from basf2 import *
from modularAnalysis import *
from stdFSParticles import *

set_log_level(LogLevel.INFO)
# BASF2 version for gbasf2
gb2_setuprel = 'release-00-07-00'

# Input file name (using inputMdstList for a list of file names)
# Can be overwritten by external arguement
inputMdst('/belle/MC/fab/signal/B2JpsiKs_mu/mcprod1503/mc46_B2JpsiKs_mu_BGx0_fab/s00/B2JpsiKs_mu_e0010r0091_s00_BGx0.mdst.root')

# Load final state paricles
stdFSParticles()
# Load skim lists
from B2hhh_List import *

b2hhhList = B2hhh_List()
skimOutputUdst('CharmlessB2hhh_Skim', b2hhhList)
summaryOfLists(b2hhhList)

process(analysis_main)

# print out the summary
print(statistics)
