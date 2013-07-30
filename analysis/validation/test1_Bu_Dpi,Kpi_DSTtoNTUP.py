#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
from basf2 import *
from modularAnalysis import *

inputMdst('../Bu_D0pi,Kpi_GENSIMRECtoDST.dst.root')
loadReconstructedParticles()

selectParticle('K-', -321, [''])
selectParticle('pi+', 211, [''])
selectParticle('pi-', -211, [''])

makeParticle(
    'D0',
    421,
    ['K-', 'pi+'],
    1.7,
    2.0,
    )
applyCuts('D0', ['M 1.81:1.91'])

# Prepare the B candidates
makeParticle(
    'B-toD0pi',
    -521,
    ['D0', 'pi-'],
    5.2,
    5.4,
    )
applyCuts('B-toD0pi', ['M 5.2:5.4'])

# ----> NtupleMaker module
ntuple1 = register_module('NtupleMaker')
# output root file name (the suffix .root will be added automaticaly)
ntuple1.param('strFileName', '../Bu_Dpi,Kpi.ntup.root')
# Name of the output TTree
ntuple1.param('strTreeName', 'Bu_Dpi_tuple')
# particle list with reconstructed decays
ntuple1.param('strListName', 'B-toD0pi')
# Specifiy output tools
# decay descriptor strings ignored (parser for  not yet implemented)
ntuple1.param('strTools', [  # exp_no, run_no, evt_no
    'EventMetaData',
    'B-',
    'RecoStats',
    'B-',
    'DeltaEMbc',
    '^B- -> (anti-D0 -> K+ pi-) pi-',
    'MCTruth',
    'B- -> (anti-D0 -> ^K+ ^pi-) ^pi-',
    'PID',
    'B- -> (anti-D0 -> ^K+ ^pi-) ^pi-',
    'Track',
    'B- -> (anti-D0 -> ^K+ ^pi-) ^pi-',
    'MCHierarchy',
    'B- -> (anti-D0 -> ^K+ ^pi-) ^pi-',
    'MCKinematics',
    'B- -> (anti-D0 -> ^K+ ^pi-) ^pi-',
    'Kinematics',
    '^B- -> (^anti-D0 -> ^K+ ^pi-) ^pi-',
    ])

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
