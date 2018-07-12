#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to write out the
# decay hash. This allows to store information on
# the reconstructed and original decay and use it
# in the offline analysis steps.
#
#   J/psi
#    |
#    +-> e- e+
#
# Notes:
#  1) No guarantee for collisions in the decayHash.
#  2) Be careful with the removeRadiativeGammaFlag
#     parameter. Bremsstrahlungsgammas are identified
#     as follows:
#     If a decay has more than 2 daughters and pdg of
#     that daughter is 22. I.e. in a decay like
#     A --> B C gamma, the gamma would be removed.
#
# Contributors: Moritz Gelb (June 2017)
#
######################################################

import root_pandas
import decayHash
from decayHash import DecayHashMap

# read in root-file as a pandas dataframe
data = root_pandas.read_root('Jpsi.root')
hashmap = DecayHashMap('hashmap_Jpsi.root', removeRadiativeGammaFlag=False)
hashmap2 = DecayHashMap('hashmap_Jpsi.root', removeRadiativeGammaFlag=True)

# get one reconstructed J/psi
candidate42 = data.iloc[42][["extraInfo__boDecayHash__bc", "extraInfo__boDecayHashExtended__bc"]].values

# print the reconstruced decay
print("Reconstructed Decay: ")
rec = hashmap.get_reconstructed_decay(*candidate42)
print(rec.to_string())

# print the original decay as simulated in MC
print("Monte Carlo Decay: ")
org = hashmap.get_original_decay(*candidate42)
print(org.to_string())

# print the original decay as simulated in MC with removed Bremstrahlung gammas
print("Monte Carlo Decay with removed Bremsstrahlung gammas: ")
org2 = hashmap2.get_original_decay(*candidate42)
print(org2.to_string())

# search for a specific decay (sub-decay)
print("Search for decay:")
search_decay = decayHash.Belle2.DecayTree('511 (--> 130 (-> -11 11 22) 443)')
print(search_decay.to_string())
found = hashmap.get_original_decay(data.iloc[42]["extraInfo__boDecayHash__bc"],
                                   data.iloc[42]["extraInfo__boDecayHashExtended__bc"]).find_decay(search_decay)
print("Found: ", found)
