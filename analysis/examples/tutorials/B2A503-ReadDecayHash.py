#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
#                                                                        #
# Stuck? Ask for help at questions.belle2.org                            #
#                                                                        #
# This tutorial demonstrates how to write out the                        #
# decay hash. This allows to store information on                        #
# the reconstructed and original decay and use it                        #
# in the offline analysis steps.                                         #
#                                                                        #
#   J/psi                                                                #
#    |                                                                   #
#    +-> e- e+                                                           #
#                                                                        #
# Notes:                                                                 #
#  1) No guarantee for collisions in the decayHash.                      #
#  2) Be careful with the removeRadiativeGammaFlag                       #
#     parameter. Bremsstrahlung gammas are identified                    #
#     as follows:                                                        #
#     If a decay has more than 2 daughters and pdg of                    #
#     that daughter is 22. I.e. in a decay like                          #
#     A -> B C gamma, the gamma would be removed.                        #
#                                                                        #
##########################################################################

import uproot
import basf2 as b2
from decayHash import DecayHashMap
import sys

# read in root-file as a pandas dataframe
data = uproot.open(b2.find_file('Jpsi_from_B2A502.root', 'examples', False))["variables"].arrays(library="pd")
hashmap = DecayHashMap(b2.find_file('hashmap_Jpsi_from_B2A502.root', 'examples', False), removeRadiativeGammaFlag=False)
hashmap2 = DecayHashMap(b2.find_file('hashmap_Jpsi_from_B2A502.root', 'examples', False), removeRadiativeGammaFlag=True)

# get one reconstructed J/psi
candidate42 = data.iloc[42][["extraInfo__boDecayHash__bc", "extraInfo__boDecayHashExtended__bc"]].values

# print the reconstructed decay
print("Reconstructed Decay: ")
rec = hashmap.get_reconstructed_decay(*candidate42)
print(rec.to_string())

# print the original decay as simulated in MC
print("Monte Carlo Decay: ")
org = hashmap.get_original_decay(*candidate42)
print(org.to_string())

# print the original decay as simulated in MC with removed Bremsstrahlung gammas
print("Monte Carlo Decay with removed Bremsstrahlung gammas: ")
org2 = hashmap2.get_original_decay(*candidate42)
print(org2.to_string())

# search for a specific decay (sub-decay)
import ROOT  # noqa
print("Search for decay:")
search_decay = ROOT.Belle2.DecayTree('511 (-> 130 (-> -11 11 22) 443)')
print(search_decay.to_string())
found = hashmap.get_original_decay(data.iloc[42]["extraInfo__boDecayHash__bc"],
                                   data.iloc[42]["extraInfo__boDecayHashExtended__bc"]).find_decay(search_decay)
print("Found: ", found)
sys.exit(0)
