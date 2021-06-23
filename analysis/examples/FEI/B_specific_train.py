#!/usr/bin/env python3

# William Sutcliffe 2019
#
# Steering file to train the specfic FEI on Belle II MC, but it can be also easily adapted for converted Belle MC.
# This steering file is called several times (so-called stages) during the training process of the FEI.
# For reference see Confluence and Thomas Keck's PhD thesis.
#
# Please adapt for your signal channel. Note that a large amount of MC is needed to train the specific FEI.
# I usually use 100 million of signal events for each final state, mixed and charged MC.
# This example is for hadronic tagging.

import fei
import basf2 as b2
import modularAnalysis as ma

# Create path
path = b2.create_path()

# Load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('mdst14.root', 'validation', False),
             path=path)

# Max 12 tracks per event - this avoids much computing time.
empty_path = b2.create_path()
skimfilter = b2.register_module('VariableToReturnValue')
skimfilter.param('variable', 'nCleanedTracks(dr < 2 and abs(dz) < 4)')
skimfilter.if_value('>12', empty_path, b2.AfterConditionPath.END)
path.add_module(skimfilter)

# Signal side reconstruction
ma.fillParticleList('mu+', 'muonID > 0.8 and dr < 2 and abs(dz) < 4', writeOut=True, path=path)
ma.fillParticleList('e+', 'electronID > 0.8 and dr < 2 and abs(dz) < 4', writeOut=True, path=path)
ma.fillParticleList(
    'gamma',
    '[[clusterReg == 1 and E > 0.10] or [clusterReg == 2 and E > 0.09] or [clusterReg == 3 and E > 0.16]]',
    writeOut=True,
    path=path)
ma.reconstructDecay(
    'B+:sig_e -> gamma e+',
    '1.000 < M < 6.000 and cos(useRestFrame(daughterAngle(0, 1))) < 0.6',
    dmID=1,
    writeOut=True,
    path=path)
ma.reconstructDecay(
    'B+:sig_mu -> gamma mu+',
    '1.000 < M < 6.000 and cos(useRestFrame(daughterAngle(0, 1))) < 0.6',
    dmID=2,
    writeOut=True,
    path=path)
ma.copyLists('B+:sig', ['B+:sig_e', 'B+:sig_mu'], writeOut=True, path=path)
ma.looseMCTruth('B+:sig', path=path)
ma.rankByHighest('B+:sig', 'daughter(0,E)', outputVariable='PhotonCandidateRank', path=path)
ma.buildRestOfEvent('B+:sig', path=path)
clean_roe_mask = (
    'CleanROE',
    'dr < 2 and abs(dz) < 4',
    'clusterE9E25 > 0.9 and clusterTiming < 50 and E > 0.09 and trackMatchType==0')
ma.appendROEMasks('B+:sig', [clean_roe_mask], path=path)
ma.applyCuts('B+:sig', 'roeDeltae(CleanROE) < 2.0 and roeMbc(CleanROE) > 4.8', path=path)

skimfilter = b2.register_module('SkimFilter')
skimfilter.param('particleLists', ['B+:sig'])
empty_path = b2.create_path()
skimfilter.if_value('=0', empty_path, b2.AfterConditionPath.END)
path.add_module(skimfilter)

# Prepare list for the training.
path.add_module('MCDecayFinder', decayString='B+ -> e+ nu_e gamma', listName='B+:FEIMC_e', writeOut=True)
path.add_module('MCDecayFinder', decayString='B+ -> mu+ nu_mu gamma', listName='B+:FEIMC_mu', writeOut=True)
ma.copyLists('B+:FEIMC', ['B+:FEIMC_e', 'B+:FEIMC_mu'], writeOut=True, path=path)


# We want the FEI to be only trained on a correctly reconstruced signal side and on wrongly reconstructed background.
isSignal = 'isSignalAcceptMissingNeutrino'
signalMC = 'eventCached(countInList(B+:FEIMC))'
cut = '[[{mc} > 0 and {sig} == 1] or [{mc} == 0 and {sig} != 1]]'.format(mc=signalMC, sig=isSignal)
ma.applyCuts('B+:sig', cut, path=path)

# Set up FEI configuration specifying the FEI prefix
fei_tag = 'my_specFEI'
belle_particles = fei.get_default_channels(KLong=False,
                                           chargedB=True,
                                           neutralB=True,
                                           semileptonic=False,
                                           B_extra_cut='nRemainingTracksInEvent <= 3',
                                           specific=True)

# Get FEI path
configuration = fei.config.FeiConfiguration(prefix=fei_tag, training=True, monitor=False, cache=-1)


# Add FEI path to the path to be processed
feistate = fei.get_path(belle_particles, configuration)

# FEI training
if feistate.stage == 0:
    # Write out the rest of event, we train only on the rest of event of our signal side.
    # This is the main difference compared to the generic FEI.
    rO = b2.register_module('RootOutput')
    rO.set_name('ROE_RootOutput')
    rO.param('additionalBranchNames', ['RestOfEvent'])
    feistate.path.add_module(rO)
    roe_path = b2.create_path()
    cond_module = b2.register_module('SignalSideParticleFilter')
    cond_module.param('particleLists', ['B+:sig'])
    cond_module.if_true(feistate.path, b2.AfterConditionPath.END)
    roe_path.add_module(cond_module)
    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)
else:
    # After stage 0, the training is done only on the written out rest of event.
    path = b2.create_path()
    ma.inputMdstList('default', [], path)
    path.add_path(feistate.path)
    r1 = b2.register_module('RootOutput')
    r1.set_name('ROE_RootOutput')
    r1.param('additionalBranchNames', ['RestOfEvent'])
    path.add_module(r1)


# Process 100 events
b2.process(path, max_event=100)
