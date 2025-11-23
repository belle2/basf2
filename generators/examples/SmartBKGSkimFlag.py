##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import sys

import basf2 as b2
from skim.WGs.fei import feiHadronicB0
import modularAnalysis as ma
import mdst

from smartBKG.b2modules.NN_trainer_module import SaveFlag

# get job array id from batch job manager (slurm)
job_id = int(sys.argv[1])

num_events = 1000
out_dir = "./"
f_input = "./mdst.root"

main = b2.create_path()

# input Mdst, skip num_events*job_id events used by previous batch jobs
ma.inputMdst(environmentType="default", filename=f_input, skipNEvents=num_events*job_id, path=main)

# Create the mDST output file before skimming, which will be used in the preprocessing.
# Not necessary if the whole input mdst is used in one job.
mdst.add_mdst_output(
    path=main,
    filename=f'{out_dir}_submdst{job_id}.root',
)

# Arbitrary skimming process, here Hadronic B0 FEI as an example
skim = feiHadronicB0(analysisGlobaltag=ma.getAnalysisGlobaltag(), udstOutput=False)
skim(main)
ma.applyEventCuts("eventExtraInfo(passes_feiHadronicB0)", main)
# Save the event number of each pass event as the flag for the training of NN
main.add_module(SaveFlag(f'{out_dir}_flag{job_id}.parquet'))

b2.process(path=main, max_event=num_events)
