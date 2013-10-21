#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
from basf2 import *
from modularAnalysis import *

inputMdst('../Bd_Kstgamma_GENSIMRECtoDST.dst.root')
loadReconstructedParticles()

selectParticle('K-', -321, [''])
selectParticle('pi+', 211, [''])
selectParticle('gamma', 22, [''])
makeParticle(
    'K*0',
    313,
    ['K-', 'pi+'],
    0.6,
    1.2,
    )

# Prepare the B candidates
makeParticle(
    'B0toK*0gamma',
    511,
    ['K*0', 'gamma'],
    5.2,
    5.4,
    )

# ----> NtupleMaker module
ntuple1 = register_module('NtupleMaker')
# output root file name (the suffix .root will be added automaticaly)
ntuple1.param('strFileName', '../Bd_Kstgamma.ntup.root')
ntuple1.param('strTreeName', 'Bd_Kstgamma_tuple')
ntuple1.param('strListName', 'B0toK*0gamma')
ntuple1.param('strTools', [
    'EventMetaData',
    'B0',
    'RecoStats',
    'B0',
    'Kinematics',
    '^B0 -> [^K*0->^K+ ^pi-] ^gamma',
    'MCTruth',
    'B0 -> [K*0->^K+ ^pi-] ^gamma',
    'DeltaEMbc',
    '^B0 -> [K*0->K+ pi-] gamma',
    'MCHierarchy',
    'B0 -> [K*0->^K+ ^pi-] ^gamma',
    'PID',
    'B0 -> [K*0->^K+ ^pi-] gamma',
    ])
    # exp_no, run_no, evt_no
main.add_module(ntuple1)

# useful to save some event summary information
ntuple2 = register_module('NtupleMaker')
ntuple2.param('strTreeName', 'eventTuple')
ntuple2.param('strListName', '')
ntuple2.param('strTools', ['EventMetaData', 'B-'])
main.add_module(ntuple2)

# ----> start processing of modules
process(main)

# ----> Print call statistics
print statistics
