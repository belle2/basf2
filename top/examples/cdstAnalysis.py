#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# -------------------------------------------------------------------------
# Example of using top variables on real data (cdst files required)
# See topVariables.py on how to use top variables on MC
#
# To print available top variables: basf2 top/examples/printTOPVariables.py
# -------------------------------------------------------------------------

from basf2 import conditions, create_path, process, statistics
from reconstruction import prepare_user_cdst_analysis
import modularAnalysis as ma
from variables import variables

# global tags
# ******************************************************************************************************************
# note: The patching global tags and their order are bucket number and basf2 version dependent.
#       Given below is what is needed for cdst files of bucket 16 calibration and January-2023 development version.
# ******************************************************************************************************************
conditions.override_globaltags()
conditions.append_globaltag('patch_main_release-07_noTOP')
conditions.append_globaltag('data_reprocessing_proc13')  # experiments 7 - 18
# conditions.append_globaltag('data_reprocessing_prompt')  # experiments 20 - 26
conditions.append_globaltag('online')

# create path
main = create_path()

# read events from a cdst file: use -i option to pass the name of the file
# files of bucket 16 can be found on KEKCC in /gpfs/group/belle2/dataprod/Data/PromptReco/bucket16_calib/
main.add_module('RootInput')

# run unpackers and post-tracking reconstruction
prepare_user_cdst_analysis(main)

# make a particle list of pions from all charged tracks
ma.fillParticleList(decayString='pi+:all', cut='', path=main)

# make aliases of some expert variables
variables.addAlias('topTOF_kaon', 'topTOFExpert(321)')

# define a list of variables to be written to ntuple
var_list = ['p',
            'theta',
            'phi',
            'charge',
            'PDG',
            'topSlotID',
            'topLocalX',
            'topLocalY',
            'topLocalZ',
            'topLocalPhi',
            'topLocalTheta',
            'topTOF',
            'topTOF_kaon',
            'extrapTrackToTOPimpactZ',
            'extrapTrackToTOPimpactTheta',
            'extrapTrackToTOPimpactPhi',
            'topDigitCount',
            'topDigitCountSignal',
            'topDigitCountBkg',
            'topDigitCountRaw',
            'topLogLFlag',
            'topLogLPhotonCount',
            'topLogLExpectedPhotonCount',
            'topLogLEstimatedBkgCount',
            'topLogLElectron',
            'topLogLMuon',
            'topLogLPion',
            'topLogLKaon',
            'topLogLProton',
            'topLogLDeuteron',
            'topBunchIsReconstructed',
            'topBunchNumber',
            'topBunchOffset',
            'topBunchTrackCount',
            'topBunchUsedTrackCount',
            'topTracksInSlot']

# write variables to ntuple
ma.variablesToNtuple(decayString='pi+:all', variables=var_list, treename='tree', filename='topVars_data.root', path=main)

# print progress
main.add_module('Progress')

# process events
process(main)

# Print statistics
print(statistics)
