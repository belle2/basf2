#!/bin/bash
# example workflow file to generate & simulate events
# and run the hlt and the offlin reconstruction 

# generate and simulate events
basf2 hlt/hlt/examples/prod_gen_sim.py
# hlt reco and only the HLTTags are stored to the root output file
basf2 hlt/hlt/examples/prod_hlt.py
# uses the gen+sim for offline reco and combines with the HLTTags from the previous
# steering file
basf2 hlt/hlt/examples/prod_reco.py
