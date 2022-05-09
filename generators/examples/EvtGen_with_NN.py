import sys

# Custom functions
import basf2 as b2
import generators as ge
import simulation as si
import reconstruction as re
from skim.WGs.fei import feiHadronicB0
import modularAnalysis as ma
import glob
import mdst

from smartBKG.models.GAT_apply_module import GATApplyModule

# according to official MC14 mixed script
# https://stash.desy.de/projects/B2P/repos/mc/browse/MC14/release-05-02-00/DB00001330/4S/mixed/mixed_eph3.py
num_events = 100
out_dir = "./"

# used for slurm batch, get job array id from slurm input
try:
    job_id = int(sys.argv[1])
except BaseException:
    job_id = ''

# background (collision) files
bg = glob.glob('./beambg*.root')
# background if running locally
bg_local = glob.glob('/group/belle2/dataprod/BGOverlay/early_phase3/prerelease-05-00-00a/overlay/phase31/BGx1/set0/*.root')

# set additional database conditions for MC14a generation
b2.conditions.prepend_globaltag("mc_production_MC14a")

# create path
main = b2.create_path()

# default to early phase 3 (exp=1003), run 0
main.add_module("EventInfoSetter", expList=1003, runList=0, evtNumList=num_events)

# generate BBbar events
ge.add_evtgen_generator(path=main, finalstate='mixed')

# GAT prediction
GATApplyModule_m = GATApplyModule(
        extra_info_var='GAT_AfterGen'
    )
dead_path = b2.create_path()
GATApplyModule_m.if_false(dead_path)
main.add_module(GATApplyModule_m)

# Create the mDST output file directly after EvtGen, can be used for training NN
mdst.add_mdst_output(
    path=main,
    filename=f'{out_dir}AfterGen{job_id}.root',
    additionalBranches=["EventExtraInfo"]
)

# detector simulation
si.add_simulation(path=main, bkgfiles=bg)

# reconstruction
re.add_reconstruction(path=main)

# FEI skims from Leptonic WG
fei_skim = feiHadronicB0(udstOutput=False, analysisGlobaltag=ma.getAnalysisGlobaltag())
fei_skim(path=main)

# Create the mDST output file after Skim
mdst.add_mdst_output(
    path=main,
    filename=f'{out_dir}AfterFEI{job_id}.root',
    additionalBranches=["EventExtraInfo"]
)

b2.process(path=main)
print(b2.statistics)
