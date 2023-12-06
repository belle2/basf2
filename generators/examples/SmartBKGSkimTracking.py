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
file_id = int((job_id - 1)/73+1)

num_events = 2500
out_dir = "./"
f_input = f"./mdst{file_id}.root"

main = b2.create_path()

# input Mdst, skip num_events*(job_id-1) events used by previous jobs
# job_id MUST at least start from 1
ma.inputMdst(environmentType="default", filename=f_input, skipNEvents=num_events*((job_id-1) % 73), path=main)

# Create the mDST output file before skimming
mdst.add_mdst_output(
    path=main,
    filename=f'{out_dir}_submdst{job_id}.root',
)

# Arbitrary skimming process, here Hadronic B0 FEI as an example
skim = feiHadronicB0(analysisGlobaltag=ma.getAnalysisGlobaltag(), udstOutput=False)
skim(main)
ma.applyEventCuts("eventExtraInfo(passes_feiHadronicB0)", main)

main.add_module(SaveFlag(f'{out_dir}_flag{job_id}.parquet'))

b2.process(path=main, max_event=num_events)
