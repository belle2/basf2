#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Moritz Gelb 2017
#
# Steering file to train the specfic FEI on Belle II MC, but it can be also easily adapted for converted Belle MC.
# This steering file is called several times (so-called stages) during the training process of the FEI.
# For reference see Confluence and Thomas Keck's PhD thesis.
#
# Please adapt for your signal channel. Note that a large amount of MC is needed to train the specific FEI.
# I usually use 100 million of signal events for each final state, mixed and charged MC.
# This example is for hadronic tagging.

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

# Prepare list for the training.
path.add_module('MCDecayFinder', decayString='B+ ==> e+ nu_e gamma', listName='B+:FEIMC_e', writeOut=True)
path.add_module('MCDecayFinder', decayString='B+ ==> mu+ nu_mu gamma', listName='B+:FEIMC_mu', writeOut=True)
copyLists('B+:FEIMC', ['B+:FEIMC_e', 'B+:FEIMC_mu'], writeOut=True, path=path)


# We want the FEI to be only trained on a correctly reconstruced signal side and on wrongly reconstructed background.
isSignal = 'isSignalAcceptMissingNeutrino'
signalMC = 'eventCached(countInList(B+:FEIMC))'
cut = '[[{mc} > 0 and {sig} == 1] or [{mc} == 0 and {sig} != 1]]'.format(mc=signalMC, sig=isSignal)
applyCuts('B+:sig', cut, path=path)

# FEI config
fei_tag = 'my_specFEI'
belle_particles = fei.get_default_channels(KLong=False,
                                           chargedB=True,
                                           neutralB=True,
                                           semileptonic=False,
                                           B_extra_cut='nRemainingTracksInEvent <= 3',
                                           specific=True)

configuration = fei.config.FeiConfiguration(prefix=fei_tag, training=True, monitor=False)
feistate = fei.get_path(belle_particles, configuration)

# FEI training
if feistate.stage == 0:
    # Write out the rest of event, we train only on the rest of event of our signal side.
    # This is the main difference compared to the generic FEI.
    rO = register_module('RootOutput')
    rO.set_name('ROE_RootOutput')
    rO.param('additionalBranchNames', ['RestOfEvent'])
    feistate.path.add_module(rO)
    roe_path = create_path()
    cond_module = register_module('SignalSideParticleFilter')
    cond_module.param('particleLists', ['B+:sig'])
    cond_module.if_true(feistate.path, AfterConditionPath.END)
    roe_path.add_module(cond_module)
    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)
else:
    # After stage 0, the training is done only on the written out rest of event.
    path = create_path()
    inputMdstList('MC7', [], path)
    path.add_path(feistate.path)
    r1 = register_module('RootOutput')
    r1.set_name('ROE_RootOutput')
    r1.param('additionalBranchNames', ['RestOfEvent'])
    path.add_module(r1)


process(path)
