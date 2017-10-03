#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Moritz Gelb 2017

# Steering file to apply the specfic FEI on Belle II MC, but it can be also easily adapted for converted Belle MC.
# For reference see Confluence and Thomas Keck's PhD thesis.
#
# Please adapt for your signal channel.
# This example is for hadronic tagging.


import os
import fei
from modularAnalysis import *


path = create_path()

# Input
inputMdstList('MC7', [], path)

# Max 12 tracks per event - this avoids much computing time.
empty_path = create_path()
skimfilter = register_module('VariableToReturnValue')
skimfilter.param('variable', 'nCleanedTracks(dr < 2 and abs(dz) < 4)')
skimfilter.if_value('>12', empty_path, AfterConditionPath.END)
path.add_module(skimfilter)

# Signal side reconstruction
fillParticleList('mu+', 'muid > 0.8 and dr < 2 and abs(dz) < 4', writeOut=True, path=path)
fillParticleList('e+', 'eid > 0.8 and dr < 2 and abs(dz) < 4', writeOut=True, path=path)
fillParticleList('gamma', 'goodGamma == 1 and E >= 1.0', writeOut=True, path=path)
reconstructDecay(
    'B+:sig_e -> gamma e+',
    '1.000 < M < 6.000 and useRestFrame(daughterAngle(0, 1)) < 0.6',
    dmID=1,
    writeOut=True,
    path=path)
reconstructDecay(
    'B+:sig_mu -> gamma mu+',
    '1.000 < M < 6.000 and useRestFrame(daughterAngle(0, 1)) < 0.6',
    dmID=2,
    writeOut=True,
    path=path)
copyLists('B+:sig', ['B+:sig_e', 'B+:sig_mu'], writeOut=True, path=path)
looseMCTruth('B+:sig', path=path)
rankByHighest('B+:sig', 'daughter(0,E)', outputVariable='PhotonCandidateRank', path=path)
buildRestOfEvent('B+:sig', path=path)
clean_roe_mask = ('CleanROE', 'dr < 2 and abs(dz) < 4',
                  'clusterE9E25 > 0.9 and clusterTiming < 50 and goodGamma == 1 and trackMatchType==0')
appendROEMasks('B+:sig', [clean_roe_mask], path=path)
applyCuts('B+:sig', 'ROE_deltae(CleanROE) < 2.0 and ROE_mbc(CleanROE) > 4.8', path=path)

skimfilter = register_module('SkimFilter')
skimfilter.param('particleLists', ['B+:sig'])
empty_path = create_path()
skimfilter.if_value('=0', empty_path, AfterConditionPath.END)
path.add_module(skimfilter)


# FEI config
fei_tag = 'my_specFEI'
fei_dir = os.path.join('/path/to/fei', fei_tag)
use_local_database(os.path.join(fei_dir, 'localdb/database.txt'), os.path.join(fei_dir, 'localdb'), True, LogLevel.WARNING)
belle_particles = fei.get_default_channels(KLong=False,
                                           chargedB=True,
                                           neutralB=False,
                                           semileptonic=False,
                                           B_extra_cut='nRemainingTracksInEvent <= 3',
                                           specific=True)

configuration = fei.config.FeiConfiguration(prefix=fei_tag, training=False, monitor=False)
feistate = fei.get_path(belle_particles, configuration)


# Run the tagging and copy the lists
roe_path = feistate.path
empty_path = create_path()
copyLists('B-:generic_final', [], writeOut=True, path=path)
copyLists('B-:generic_final', ['B-:generic'], writeOut=True, path=roe_path)
signalSideParticleFilter('B+:sig', '', roe_path, empty_path)
path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

# Reconstruct the Upsilon
upsilon_cut = '7.5 <= M <= 10.5 and -2.0 <= missingMass <= 4.0 and -0.15 <= daughter(0,deltaE) <= 0.1'
reconstructDecay('Upsilon(4S):hadronic -> B-:generic_final B+:sig', upsilon_cut, dmID=1, path=path)
copyLists('Upsilon(4S):all', ['Upsilon(4S):hadronic'], path=path)
looseMCTruth('Upsilon(4S):all', path=path)

buildRestOfEvent('Upsilon(4S):all', path=path)
upsilon_roe = ('UpsilonROE', 'dr < 2 and abs(dz) < 4', 'goodBelleGamma == 1')
appendROEMasks('Upsilon(4S):all', [upsilon_roe], path=path)
applyCuts('Upsilon(4S):all', '-2.0 < missingMass < 4.0', path=path)
applyCuts('Upsilon(4S):all', 'ROE_eextra(UpsilonROE) <= 0.9', path=path)
applyCuts('Upsilon(4S):all', 'nROETracks(UpsilonROE) <= 4', path=path)

# Best candidate selection - only one candidate per event
rankByHighest('Upsilon(4S):all', 'daughter(0, extraInfo(SignalProbability))', numBest=1,
              outputVariable='FEIProbabilityRank', path=path)

# Write Ntuples
variablesToNTuple('Upsilon(4S):all', ['M', 'missingMass', 'E'], filename="Upsilon.root", path=path)

process(path)
