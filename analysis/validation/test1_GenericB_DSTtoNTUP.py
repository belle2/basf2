#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
from basf2 import *
from modularAnalysis import *

inputMdst('../GenericB_GENSIMRECtoDST.dst.root')
loadReconstructedParticles()

selectParticle('K-', -321, [''])
selectParticle('pi+', 211, [''])
selectParticle('pi-', -211, [''])
selectParticle('pi0', 111, [''])
selectParticle('gamma', 22, [''])
selectParticle('e+', 11, [''])
selectParticle('e-', -11, [''])
selectParticle('mu+', 13, [''])
selectParticle('mu-', -13, [''])

makeParticle(
    'K_S0',
    310,
    ['pi-', 'pi+'],
    0.4,
    0.6,
    )

# ----> NtupleMaker module
ntuple1 = register_module('NtupleMaker')
# output root file name (the suffix .root will be added automaticaly)
ntuple1.param('strFileName', '../GenericB.ntup.root')

# check PID efficiency
ntuple1.param('strTreeName', 'pituple')
ntuple1.param('strListName', 'pi+')
ntuple1.param('strTools', [
    'EventMetaData',
    '^pi+',
    'Kinematics',
    '^pi+',
    'Track',
    '^pi+',
    'MCTruth',
    '^pi+',
    'MCKinematics',
    '^pi+',
    'MCReconstructible',
    '^pi+',
    'PID',
    '^pi+',
    ])
main.add_module(ntuple1)

mcfinder = register_module('MCDecayFinder')
mcfinder.param('strDecayString', 'pi+')
mcfinder.param('strListName', 'truthpi+')
main.add_module(mcfinder)

mcfinder2 = register_module('MCDecayFinder')
mcfinder2.param('strDecayString', 'gamma')
mcfinder2.param('strListName', 'truthgamma')
main.add_module(mcfinder2)

ntuple1truth = register_module('NtupleMaker')
ntuple1truth.param('strTreeName', 'truthpituple')
ntuple1truth.param('strListName', 'truthpi+')
ntuple1truth.param('strTools', [
    'EventMetaData',
    '^pi+',
    'Kinematics',
    '^pi+',
    'Track',
    '^pi+',
    'MCTruth',
    '^pi+',
    'MCKinematics',
    '^pi+',
    'MCReconstructible',
    '^pi+',
    'PID',
    '^pi+',
    ])
main.add_module(ntuple1truth)

ntuple1b = register_module('NtupleMaker')
ntuple1b.param('strTreeName', 'gammatuple')
ntuple1b.param('strListName', 'gamma')
ntuple1b.param('strTools', [
    'EventMetaData',
    '^gamma',
    'Kinematics',
    '^gamma',
    'Track',
    '^gamma',
    'MCTruth',
    '^gamma',
    'MCKinematics',
    '^gamma',
    'PID',
    '^gamma',
    ])
main.add_module(ntuple1b)

ntuple1btruth = register_module('NtupleMaker')
ntuple1btruth.param('strTreeName', 'truthgammatuple')
ntuple1btruth.param('strListName', 'truthgamma')
ntuple1btruth.param('strTools', [
    'EventMetaData',
    '^gamma',
    'Kinematics',
    '^gamma',
    'Track',
    '^gamma',
    'MCTruth',
    '^gamma',
    'MCKinematics',
    '^gamma',
    'PID',
    '^gamma',
    ])
main.add_module(ntuple1btruth)

# check pi0 resolution
ntuple2 = register_module('NtupleMaker')
ntuple2.param('strTreeName', 'pi0tuple')
ntuple2.param('strListName', 'pi0')
ntuple2.param('strTools', [
    'EventMetaData',
    'pi0',
    'MCTruth',
    'pi0 -> ^gamma ^gamma',
    'MCKinematics',
    'pi0 -> ^gamma ^gamma',
    'Kinematics',
    '^pi0 -> ^gamma ^gamma',
    ])
main.add_module(ntuple2)

# quick search for KS0 candidates
ntuple3 = register_module('NtupleMaker')
ntuple3.param('strTreeName', 'kstuple')
ntuple3.param('strListName', 'K_S0')
ntuple3.param('strTools', [
    'EventMetaData',
    'K_S0',
    'MCTruth',
    '^K_S0 -> ^pi+ ^pi-',
    'Kinematics',
    '^K_S0 -> ^pi+ ^pi-',
    ])
main.add_module(ntuple3)

# dump all event summary information
ntuple4 = register_module('NtupleMaker')
ntuple4.param('strTreeName', 'eventTuple')
ntuple4.param('strListName', '')
ntuple4.param('strTools', [
    'EventMetaData',
    'B-',
    'RecoStats',
    'B-',
    'DetectorStatsRec',
    'B-',
    'DetectorStatsSim',
    'B-',
    ])
main.add_module(ntuple4)

# ----> start processing of modules
process(main)

# ----> Print call statistics
print statistics
