##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2 as b2
import generators as ge
import simulation as si
import reconstruction as re
from skim.WGs.fei import feiHadronicB0
import modularAnalysis as ma
import mdst

from smartBKG.NN_filter_module import NNFilterModule

num_events = 1000
out_dir = "./"

# create path
main = b2.create_path()

# default to early phase 3 (exp=1003), run 0
main.add_module("EventInfoSetter", expList=1003, runList=0, evtNumList=num_events)

# generate BBbar events
ge.add_evtgen_generator(path=main, finalstate='mixed')

# GAT prediction
NNFilterModule_m = NNFilterModule(
        extra_info_var='GAT_AfterGen'
    )
dead_path = b2.create_path()
NNFilterModule_m.if_false(dead_path)
main.add_module(NNFilterModule_m)

# Create the mDST output file directly after EvtGen, can be used for training NN
mdst.add_mdst_output(
    path=main,
    filename=f'{out_dir}AfterGen.root',
    additionalBranches=["EventExtraInfo"]
)

# detector simulation
si.add_simulation(path=main)

# reconstruction
re.add_reconstruction(path=main)

# FEI skims from Leptonic WG
fei_skim = feiHadronicB0(udstOutput=False, analysisGlobaltag=ma.getAnalysisGlobaltag())
fei_skim(path=main)

# Create the mDST output file after Skim
mdst.add_mdst_output(
    path=main,
    filename=f'{out_dir}AfterFEI.root',
    additionalBranches=["EventExtraInfo"]
)

b2.process(path=main)
print(b2.statistics)
