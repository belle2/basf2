#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# -------------------------------------------------------------------------
# Example of using top variables on MC
# See cdstAnalysis.py on how to use top variables on real data
#
# To print available top variables: basf2 top/examples/printTOPVariables.py
# -------------------------------------------------------------------------

from basf2 import create_path, process
from simulation import add_simulation
from reconstruction import add_reconstruction
from background import get_background_files
import modularAnalysis as ma
from variables import variables

# create path
main = create_path()

# generate and reconstruct 100 generic BBbar events w/ the beam background overlayed
main.add_module('EventInfoSetter', evtNumList=[100])
main.add_module('EvtGenInput')
add_simulation(main, bkgfiles=get_background_files())
add_reconstruction(main)

# make a particle list of MC true pions
ma.fillParticleList(decayString='pi+', cut='', path=main)
ma.matchMCTruth(list_name='pi+', path=main)
ma.applyCuts(list_name='pi+', cut='isSignal==1', path=main)

# make aliases of some expert variables
variables.addAlias('topTOF_kaon', 'topTOFExpert(321)')
variables.addAlias('topLogLPhotonCountMCMatch', 'topDigitCountIntervalMCMatch(-20, 74.327756)')

# define a list of variables to be written to ntuple
var_list = ['p',
            'theta',
            'phi',
            'charge',
            'PDG',
            'isSignal',
            'topSlotID',
            'topSlotIDMCMatch',
            'topLocalX',
            'topLocalY',
            'topLocalZ',
            'topLocalXMCMatch',
            'topLocalYMCMatch',
            'topLocalZMCMatch',
            'topLocalPhi',
            'topLocalTheta',
            'topLocalPhiMCMatch',
            'topLocalThetaMCMatch',
            'topTOF',
            'topTOFMCMatch',
            'topTOF_kaon',
            'extrapTrackToTOPimpactZ',
            'extrapTrackToTOPimpactTheta',
            'extrapTrackToTOPimpactPhi',
            'topDigitCount',
            'topDigitCountMCMatch',
            'topDigitCountSignal',
            'topDigitCountBkg',
            'topDigitCountRaw',
            'topLogLFlag',
            'topLogLPhotonCount',
            'topLogLPhotonCountMCMatch',
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
            'topBunchMCMatch',
            'topBunchOffset',
            'topBunchTrackCount',
            'topBunchUsedTrackCount',
            'topTracksInSlot']

# write variables to ntuple
ma.variablesToNtuple(decayString='pi+', variables=var_list, treename='tree', filename='topVars_mc.root', path=main)

# print progress
main.add_module('Progress')

# process events
process(main)
