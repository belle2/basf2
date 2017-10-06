#!/bin/bash

pdgCodes=(13 211 321 2212)

for pdgCode in "${pdgCodes[@]}"; do
	basf2 13_trackingEfficiency_createPlots.py --arg --input-file=/home/frank/tuples/trackingEfficiency_pdg_${pdgCode}_pt_*.root --arg --output-file=/home/frank/ECLTrackingValidation_${pdgCode}.root
done
