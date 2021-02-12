#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Simple example script to reconstruct Dstart for the purpose of ARICH PID performance studies
Usage : basf2 reconstruct_Dstar.py exp start_run end_run
set up to use D* skim from proc9 (adjust line 42 to change input!
"""

import os
import sys
import basf2 as b2
import modularAnalysis as ma
import variables.collections as vc
import variables.utils as vu
import stdCharged as stdc

# create path
my_path = b2.create_path()

# load input ROOT file

argvs = sys.argv
exp = argvs[1].zfill(4)
start_run = argvs[2]
end_run = argvs[3]

# Set the global log level
# set_log_level(LogLevel.INFO)

# set specific database tag
# b2.conditions.override_globaltags(["tagname"])
# use local database
# b2.conditions.testing_payloads = ["localdb/database.txt"]

main_path = b2.create_path()

inputFilename = []

for i in range(int(start_run), int(end_run) + 1):
    ru = str(i)
    fname = '/group/belle2/dataprod/Data/release-03-02-02/DB00000654/proc9/e' + exp + '/4S/r' + \
        ru.zfill(5) + '/offskim/offskim_dstar/cdst/sub00/cdst.physics.' + exp + '.' + ru.zfill(5) + '.HLT0.offskim_dstar.root'
    if os.path.exists(fname):
        inputFilename.append(fname)
        print(fname)

input_module = b2.register_module('RootInput')
input_module.param('inputFileNames', inputFilename)
# in case you want to reprocess arich data uncomment the line below
# input_module.param('excludeBranchNames', ['ARICHTracks','ARICHLikelihoods','ARICHTracksToExtHits','TracksToARICHLikelihoods'])
my_path.add_module(input_module)

# uncomment the lines below to re-run arich reconstruction
# arichHits = b2.register_module('ARICHFillHits')
# arichHits.param('MagFieldCorrection',1)
# my_path.add_module(arichHits)

# reconstruction from ARICHHits
# arichRecon = b2.register_module('ARICHReconstructor')
# store cherenkov angle information
# arichRecon.param('storePhotons', 1)
# my_path.add_module(arichRecon)


# use standard final state particle lists
# creates "pi+:all" ParticleList (and c.c.)
stdc.stdPi(listtype='all', path=my_path)
stdc.stdK(listtype='all', path=my_path)

# reconstruct D0 -> K- pi+ decay
# keep only candidates with 1.8 < M(Kpi) < 1.9 GeV
ma.reconstructDecay(decayString='D0:kpi -> K-:all pi+:all', cut='1.8 < M < 1.9', path=my_path)

# vertex fitting for D0
# vtx.vertexRave('D0:kpi', 0.0, path=my_path)

# reconstruct D*+ -> D0 pi+ decay
# keep only candidates with Q = M(D0pi) - M(D0) - M(pi) < 20 MeV
ma.reconstructDecay(decayString='D*+ -> D0:kpi pi+:all', cut='0.0 < Q < 0.2', path=my_path)

# vertex fitting for D*
# vtx.vertexRave('D*+:my', 0.0, path=my_path)

pidVars = [
    'd0',
    'z0',
    'cosTheta',
    'nCDCHits',
    'pidPairProbabilityExpert(321, 211, ARICH)',
    'pidDeltaLogLikelihoodValueExpert(211,321,ARICH)',
    'pidProbabilityExpert(211, ARICH)',
    'pidProbabilityExpert(321, ARICH)']

# Select variables that we want to store to ntuple
dstar_vars = vc.inv_mass + vc.mc_truth + vc.kinematics

fs_hadron_vars = vu.create_aliases_for_selected(
    list_of_variables=vc.pid + vc.track + vc.mc_truth + vc.kinematics + pidVars,
    decay_string='D*+ -> [D0 -> ^K- ^pi+] ^pi+')

d0_vars = vu.create_aliases_for_selected(
    list_of_variables=vc.inv_mass + vc.mc_truth,
    decay_string='D*+ -> ^D0 pi+',
    prefix='D0')

# Saving variables to ntuple
output_file = 'Dstar2D0Pi-PID_exp' + exp + '_r' + start_run + '-' + end_run + '.root'
ma.variablesToNtuple('D*+', dstar_vars + d0_vars + fs_hadron_vars,
                     filename=output_file, treename='dsttree', path=my_path)


progress = b2.register_module('Progress')
my_path.add_module(progress)

b2.process(my_path)
