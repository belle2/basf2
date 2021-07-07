##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import os

# merges histograms from all files in g4sim/.../beast_hist

folders = ["phase2.1.3_june_Touschek/EM_2"]
base = "/group/belle2/BGcampaigns/g4sim/"
ens = ["LER", "HER"]
types = ["Touschek", "Coulomb", "Brems"]
# types = ["BHWideLargeAngle","BHWide"]
# types = ["twoPhoton"]

for folder in folders:
    for en in ens:
        for typee in types:
            outfile = base + folder + "/beast_hist/" + typee + "_" + en + "_all.root"
            infiles = base + folder + "/beast_hist/" + typee + "_" + en + "_*.root"
#            outfile =  base+folder + "/beast_hist/" + typee + "_all.root"
#            infiles = base+folder + "/beast_hist/" + typee +  "_*.root"
            os.system("rm -rf " + outfile)
            com = "hadd " + outfile + " " + infiles
            print(com)
            os.system(com)
