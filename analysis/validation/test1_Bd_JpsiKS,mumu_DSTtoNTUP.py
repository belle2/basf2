#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
from basf2 import *

from reconstruction import add_reconstruction
from modularAnalysis import *

# Create main path
main = create_path()

input = register_module('RootInput')
input.param('inputFileName', '../Bd_JpsiKS,mumu_GENSIMRECtoDST.dst.root')
main.add_module(input)

# ---------------------------------------------------------------
# Show progress of processing
progress = register_module('Progress')
gearbox = register_module('Gearbox')
main.add_module(progress)
main.add_module(gearbox)

# ----------------------------------------------------------------
loadReconstructedParticles(main)

selectParticle(main, 'pi+', 211, [''])
selectParticle(main, 'pi-', -211, [''])
selectParticle(main, 'mu+', 13, [''])
selectParticle(main, 'mu-', -13, [''])

makeParticle(
    main,
    'jpsi',
    443,
    ['mu-', 'mu+'],
    2.8,
    3.3,
    )
makeParticle(
    main,
    'KS0',
    310,
    ['pi-', 'pi+'],
    0.4,
    0.6,
    )

# Prepare the B candidates
makeParticle(
    main,
    'B0toJpsiKS',
    511,
    ['jpsi', 'KS0'],
    5.2,
    5.4,
    )

# ----> NtupleMaker module
ntuple1 = register_module('NtupleMaker')
# output root file name (the suffix .root will be added automaticaly)
ntuple1.param('strFileName', '../Bd_JpsiKS,mumu.ntup.root')
ntuple1.param('strTreeName', 'Bd_JpsiKS_tuple')
ntuple1.param('strListName', 'B0toJpsiKS')
ntuple1.param('strTools', [
    'EventMetaData',
    'B0',
    'RecoStats',
    'B0',
    'Kinematics',
    '^B0 -> (^J/Psi->^mu+ ^mu-) (^KS0 -> ^pi+ ^pi-)',
    'MCTruth',
    'B0 -> (J/Psi->^mu+ ^mu-) (KS0 -> ^pi+ ^pi-)',
    'DeltaEMbc',
    '^B0 -> (J/Psi->mu+ mu-) (KS0 -> pi+ pi-)',
    'MCHierarchy',
    'B0 -> (J/Psi->^mu+ ^mu-) (KS0 -> ^pi+ ^pi-)',
    'PID',
    'B0 -> (J/Psi->^mu+ ^mu-) (KS0 -> ^pi+ ^pi-)',
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
