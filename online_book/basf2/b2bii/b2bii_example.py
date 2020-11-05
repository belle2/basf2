#!/usr/bin/env python3

import os

import basf2 as b2
import modularAnalysis as ma
import variables as va
import variables.collections as vc
import variables.utils as vu
from b2biiConversion import convertBelleMdstToBelleIIMdst

os.environ["USE_GRAND_REPROCESS_DATA"] = "1"
os.environ["PGUSER"] = "g0db"

# Convert
main = b2.create_path()
inputfile = b2.find_file(
    "b2bii_input_evtgen_exp_07_BptoD0pip-D0toKpipi0-0.mdst", "examples", False
)
convertBelleMdstToBelleIIMdst(inputfile, path=main)

# Only charged final state particles need to be loaded. The neutral particles
# gamma, pi0, K_S0, K_L0, and Lambda0 are already loaded to the 'gamma:mdst',
# 'pi0:mdst', 'K_S0:mdst', 'K_L0:mdst', and 'Lambda0:mdst' particle lists,
# respectively.
ma.fillParticleList("pi+:sig", "atcPIDBelle(3,2)<0.4", path=main)
ma.fillParticleList("K+:sig", "atcPIDBelle(3,2)>0.6", path=main)

# The Belle PID variables are: atcPIDBelle(sigHyp, bkgHyp), muIDBelle, and eIDBelle
va.variables.addAlias("Lkpi", "atcPIDBelle(3,2)")

# Now, let's really reconstruct a B decay with an intermediate D meson:
ma.reconstructDecay(
    "D0:Kpipi0 -> K-:sig pi+:sig pi0:mdst", cut="1.7 < M < 2.0", path=main
)
ma.reconstructDecay(
    "B+:D0pi -> anti-D0:Kpipi0 pi+:sig",
    cut="4.8 < Mbc >5.2 and abs(deltaE) < 0.15",
    path=main,
)

ma.matchMCTruth("B+:D0pi", path=main)

# create and fill flat Ntuple with MCTruth and kinematic information
# Create list of variables to save into the output file
b_vars = []

standard_vars = vc.kinematics + vc.mc_kinematics + vc.mc_truth + vc.inv_mass
b_vars += vc.deltae_mbc
b_vars += standard_vars

# Variables for D0 and all final state particles (kaons, pions, neutral pions)
b_vars += vu.create_aliases_for_selected(
    standard_vars,
    "B+ -> [ ^D0 -> ^K- ^pi+ ^pi0] ^pi+",
    prefix=["D0", "K", "pip1", "pi0", "pip2"],
)

# Variables for charged final states (kaons, pions)
belle1pid = (
    ["eIDBelle", "muIDBelleQuality", "muIDBelle", "Lkpi"]
    + vc.track
    + vc.track_hits
)

b_vars += vu.create_aliases_for_selected(
    belle1pid, "B+ -> [ D0 -> ^K- ^pi+ pi0] ^pi+", prefix=["K", "pip1", "pip2"]
)

ma.variablesToNtuple(
    "B+:D0pi", b_vars, filename="B2BII_B2D0pi_D2Kpipi0.root", path=main
)

# progress
main.add_module("Progress")

b2.process(main)

# Print call statistics
print(b2.statistics)
