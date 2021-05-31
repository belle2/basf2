#!/usr/bin/env python3

# William Sutcliffe 2019
# Mortz Gelb 2017

# Steering file to apply the specfic FEI on Belle II MC, but it can be also easily adapted for converted Belle MC.
# For reference see Confluence and Thomas Keck's PhD thesis.
#
# Please adapt for your signal channel.
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
    'clusterE9E25 > 0.9 and clusterTiming < 50 and E > 0.9 and trackMatchType==0')
ma.appendROEMasks('B+:sig', [clean_roe_mask], path=path)
ma.applyCuts('B+:sig', 'roeDeltae(CleanROE) < 2.0 and roeMbc(CleanROE) > 4.8', path=path)

skimfilter = b2.register_module('SkimFilter')
skimfilter.param('particleLists', ['B+:sig'])
empty_path = b2.create_path()
skimfilter.if_value('=0', empty_path, b2.AfterConditionPath.END)
path.add_module(skimfilter)


# FEI config prefix
# Set the prefix
# fei_tag = 'my_specFEI'

# Here we use a prefix of an existing FEI training
fei_tag = 'FEIv4_2021_MC14_release_05_01_12'

# Add the necessary database
# b2.conditions.globaltags = ['name of database containing the specific training']

# Here we use a generic FEI training to demonstrate applying the FEI in an ROE of the signal
b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

belle_particles = fei.get_default_channels(KLong=False,
                                           chargedB=True,
                                           neutralB=False,
                                           semileptonic=False,
                                           B_extra_cut='nRemainingTracksInEvent <= 3',
                                           specific=True)

configuration = fei.config.FeiConfiguration(prefix=fei_tag, training=False, monitor=False, cache=0)
feistate = fei.get_path(belle_particles, configuration)


# Run the tagging and copy the lists
roe_path = feistate.path
empty_path = b2.create_path()
ma.copyLists('B-:generic_final', [], writeOut=True, path=path)
ma.copyLists('B-:generic_final', ['B-:generic'], writeOut=True, path=roe_path)
ma.signalSideParticleFilter('B+:sig', '', roe_path, empty_path)
path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

# Reconstruct the Upsilon
upsilon_cut = '7.5 <= M <= 10.5 and -2.0 <= m2RecoilSignalSide <= 4.0 and -0.15 <= daughter(0,deltaE) <= 0.1'
ma.reconstructDecay('Upsilon(4S):hadronic -> B-:generic_final B+:sig', upsilon_cut, dmID=1, path=path)
ma.copyLists('Upsilon(4S):all', ['Upsilon(4S):hadronic'], path=path)
ma.looseMCTruth('Upsilon(4S):all', path=path)

ma.buildRestOfEvent('Upsilon(4S):all', path=path)
upsilon_roe = ('UpsilonROE', 'dr < 2 and abs(dz) < 4', 'goodBelleGamma == 1')
ma.appendROEMasks('Upsilon(4S):all', [upsilon_roe], path=path)
ma.applyCuts('Upsilon(4S):all', '-2.0 < m2RecoilSignalSide < 4.0', path=path)
ma.applyCuts('Upsilon(4S):all', 'roeEextra(UpsilonROE) <= 0.9', path=path)
ma.applyCuts('Upsilon(4S):all', 'nROE_Tracks(UpsilonROE) <= 4', path=path)

# Best candidate selection - only one candidate per event
ma.rankByHighest('Upsilon(4S):all', 'daughter(0, extraInfo(SignalProbability))', numBest=1,
                 outputVariable='FEIProbabilityRank', path=path)

# Write Ntuples
ma.variablesToNtuple('Upsilon(4S):all', ['M', 'm2RecoilSignalSide', 'E'], filename="Upsilon.root", path=path)

# Process 100 events
b2.process(path, max_event=100)
