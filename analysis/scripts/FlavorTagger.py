#!/usr/bin/env python
# -*- coding: utf-8 -*-

 # *************  Flavor Tagging   ************
 # * Authors: Fernando Abudinen, Moritz Gelb  *
 # *.....     and Thomas Keck                 *
 # * This script is needed to test            *
 # * the previously trained flavor tagger.    *
 # ********************************************

from basf2 import *
from modularAnalysis import *
import variables as mc_variables
from ROOT import Belle2
import os


class RemoveEmptyROEModule(Module):

    def event(self):
        self.return_value(0)
        roe = Belle2.PyStoreObj('RestOfEvent')
        B0 = roe.obj().getRelated('Particles')
        ParticleLists = ['e+', 'mu+', 'K+', 'pi+', 'p+']
        if mc_variables.variables.evaluate('isRestOfEventEmpty', B0) == -2:
            B2INFO('FOUND NO TRACKS OR B0 IN ROE! NOT USED FOR TRAINING! COMBINER OUTPUT IS MANUALLY SET'
                   )
            B0.addExtraInfo('B0Probability', 0.5)
            B0.addExtraInfo('B0barProbability', 0.5)
            B0.addExtraInfo('qrCombined', 0.0)
            B0.addExtraInfo('qrMC', -999)
            B0.addExtraInfo('NTracksInROE', 0)
            B0.addExtraInfo('NECLClustersInROE', 0)
            B0.addExtraInfo('NKLMClustersInROE', 0)
            self.return_value(1)


class RemoveExtraInfoModule(Module):

    def event(self):
        TrackLevelParticles = ['e+', 'mu+', 'K+', 'pi+', 'Lambda0']
        for symbol in TrackLevelParticles:
            plist = Belle2.PyStoreObj(symbol + ':ROE')
            for i in range(0, plist.obj().getListSize()):
                particle = plist.obj().getParticle(i)
                particle.removeExtraInfo()


class MoveTaggerInformationToBExtraInfoModule(Module):

    """
    Adds the flavor tagging information (q*r) from the MC and from the Combiner as ExtraInfo to the reconstructed B0 particle.
    """

    def event(self):
        roe = Belle2.PyStoreObj('RestOfEvent')
        info = Belle2.PyStoreObj('EventExtraInfo')
        someParticle = Belle2.Particle(None)
        B0Probability = info.obj().getExtraInfo('qrCombined')
        B0barProbability = 1 - info.obj().getExtraInfo('qrCombined')
        qrCombined = 2 * (info.obj().getExtraInfo('qrCombined') - 0.5)
        qrMC = 2 * (mc_variables.variables.evaluate('qrCombined',
                    someParticle) - 0.5)
        NTracksInROE = roe.obj().getNTracks()
        NECLClustersInROE = roe.obj().getNECLClusters()
        NKLMClustersInROE = roe.obj().getNKLMClusters()
        B0 = roe.obj().getRelated('Particles')
        B0.addExtraInfo('B0Probability', B0Probability)
        B0.addExtraInfo('B0barProbability', B0barProbability)
        B0.addExtraInfo('qrCombined', qrCombined)
        B0.addExtraInfo('qrMC', qrMC)
        B0.addExtraInfo('NTracksInROE', NTracksInROE)
        B0.addExtraInfo('NECLClustersInROE', NECLClustersInROE)
        B0.addExtraInfo('NKLMClustersInROE', NKLMClustersInROE)
        info.obj().removeExtraInfo()


# Directory where the weights of the trained Methods are saved
workingDirectory = os.environ['BELLE2_LOCAL_DIR'] \
    + '/analysis/data/FlavorTagging/TrainedMethods'

# Methods for Track and Event Levels
methods = [('FastBDT', 'Plugin',
           'CreateMVAPdfs:NbinsMVAPdf=100:!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3')]

# Methods for Combiner Level
methodsCombiner = [('FastBDT', 'Plugin',
                   'CreateMVAPdfs:NbinsMVAPdf=300:!H:!V:NTrees=300:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3')]
# SignalFraction to calculate probability, -2 if training signal/background ratio should be used
signalFraction = -2

TrackLevelParticleLists = [  # ('e+', 'IntermediateElectron'),
                             # ('mu+', 'IntermediateMuon'),
    ('e+', 'Electron'),
    ('mu+', 'Muon'),
    ('mu+', 'KinLepton'),
    ('K+', 'Kaon'),
    ('pi+', 'SlowPion'),
    ('pi+', 'FastPion'),
    ('Lambda0', 'Lambda'), ]

EventLevelParticleLists = [  # ('e+', 'IntermediateElectron'),
                             # ('mu+', 'IntermediateMuon'),
    ('e+', 'Electron'),
    ('mu+', 'Muon'),
    ('mu+', 'KinLepton'),
    ('K+', 'Kaon'),
    ('pi+', 'SlowPion'),
    ('pi+', 'FastPion'),
    ('pi+', 'MaximumP*'),
    ('pi+', 'FSC'),
    ('K+', 'KaonPion'),
    ('Lambda0', 'Lambda'), ]


def TrackLevel(weightFiles='B2JpsiKs_mu', path=analysis_main):
    B2INFO('TRACK LEVEL')
    ReadyMethods = 0
    for (symbol, category) in TrackLevelParticleLists:
        particleList = symbol + ':ROE'
        methodPrefixTrackLevel = weightFiles + 'TrackLevel' + category + 'TMVA'
        targetVariable = 'IsFromB(' + category + ')'

      # Select particles in ROE for different categories of flavour tagging.
        if symbol != 'Lambda0':
            fillParticleList(particleList, 'isInRestOfEvent > 0.5 and chiProb > 0.001', path=path)
      # Check if there is K short in this event
        if symbol == 'K+':
            applyCuts('K+:ROE', '0.1<Kid', path=path)  # Precut done to prevent from overtraining, might be redundant
            fillParticleList('pi+:inKaonRoe', 'isInRestOfEvent > 0.5 and chiProb > 0.001', path=path)
            reconstructDecay('K_S0:ROEKaon -> pi+:inKaonRoe pi-:inKaonRoe', '0.40<=M<=0.60', 1, path=path)
            fitVertex('K_S0:ROEKaon', 0.01, fitter='kfitter', path=path)

        if symbol == 'Lambda0':
            fillParticleList('pi+:inLambdaRoe', 'isInRestOfEvent > 0.5 and chiProb > 0.001', path=path)
            fillParticleList('p+:inLambdaRoe', 'isInRestOfEvent > 0.5 and chiProb > 0.001', path=path)
            reconstructDecay('Lambda0:ROE -> pi-:inLambdaRoe p+:inLambdaRoe', '1.00<=M<=1.23', 1, path=path)
            reconstructDecay('K_S0:ROELambda -> pi+:inLambdaRoe pi-:inLambdaRoe', '0.40<=M<=0.60', 1, path=path)
            fitVertex('Lambda0:ROE', 0.01, fitter='kfitter', path=path)
            matchMCTruth('Lambda0:ROE', path=path)
            fitVertex('K_S0:ROELambda', 0.01, fitter='kfitter', path=path)

        if not isTMVAMethodAvailable(workingDirectory + '/' + methodPrefixTrackLevel):
            B2FATAL('Flavor Tagger: ' + symbol + ' Tracklevel was not trained. Stopped')
        else:
            B2INFO('PROCESSING: applyTMVAMethod on track level')
            applyTMVAMethod(
                particleList,
                prefix=methodPrefixTrackLevel,
                expertOutputName=targetVariable,
                method=methods[0][0],
                signalFraction=signalFraction,
                workingDirectory=workingDirectory,
                path=path, )
            ReadyMethods += 1

    if ReadyMethods != len(TrackLevelParticleLists):
        return False
    else:
        return True


def EventLevel(weightFiles='B2JpsiKs_mu', path=analysis_main):
    B2INFO('EVENT LEVEL')
    ReadyMethods = 0
    for (symbol, category) in EventLevelParticleLists:
        particleList = symbol + ':ROE'
        methodPrefixEventLevel = weightFiles + 'EventLevel' + category + 'TMVA'
        targetVariable = 'IsRightClass(' + category + ')'
    # if category == 'KinLepton':
      # selectParticle(particleList,
      # 'isInElectronOrMuonCat < 0.5',
      # path=path)
        if category == 'KaonPion':
            fillParticleList(particleList, 'hasHighestProbInCat(' + particleList + ',' + 'IsFromB(Kaon)) > 0.5', path=path)
        elif category == 'FSC':
            fillParticleList(particleList, 'hasHighestProbInCat(' + particleList + ',' + 'IsFromB(SlowPion)) > 0.5', path=path)
        elif category == 'Lambda':
            applyCuts(particleList, 'hasHighestProbInCat(' + particleList + ',' + 'IsFromB(Lambda)) > 0.5', path=path)
        else:
            fillParticleList(particleList, 'hasHighestProbInCat(' + particleList + ',' + 'IsFromB(' + category + ')) > 0.5', path=path)

        if not isTMVAMethodAvailable(workingDirectory + '/'
                                     + methodPrefixEventLevel):
            B2FATAL('Flavor Tagger: ' + symbol + ' Eventlevel was not trained.')
        else:
            B2INFO('PROCESSING: applyTMVAMethod on event level')
            applyTMVAMethod(
                particleList,
                prefix=methodPrefixEventLevel,
                expertOutputName=targetVariable,
                method=methods[0][0],
                signalFraction=signalFraction,
                workingDirectory=workingDirectory,
                path=path, )
            ReadyMethods += 1

    if ReadyMethods != len(EventLevelParticleLists):
        return False
    else:
        return True


def CombinerLevel(weightFiles='B2JpsiKs_mu', path=analysis_main):
    B2INFO('COMBINER LEVEL')
    methodPrefixCombinerLevel = weightFiles + 'CombinerLevelTMVA'
    if not isTMVAMethodAvailable(workingDirectory + '/'
                                 + methodPrefixCombinerLevel):
        B2FATAL('Flavor Tagger: Combinerlevel was not trained. Stopped')
    else:
        B2INFO('Apply TMVAMethod on combiner level')
        applyTMVAMethod(
            [],
            expertOutputName='qrCombined',
            prefix=methodPrefixCombinerLevel,
            signalClass=1,
            method=methodsCombiner[0][0],
            signalFraction=signalFraction,
            workingDirectory=workingDirectory,
            path=path, )
        return True


def FlavorTagger(weightFiles='B2JpsiKs_mu', path=analysis_main):
    """
      Defines the whole flavor tagging process.
    """

    B2INFO('*** FLAVOR TAGGING ***')
    B2INFO(' ')
    B2INFO('    Working directory is: ' + workingDirectory)
    B2INFO(' ')

    roe_path = create_path()
    emptypath = create_path()

    ROEEmptyTrigger = RemoveEmptyROEModule()

    # If trigger returns 1 jump into empty path skipping further modules in roe_path
    roe_path.add_module(ROEEmptyTrigger)
    ROEEmptyTrigger.if_true(emptypath)

    # track training or expert
    TrackLevel(weightFiles, roe_path)
    EventLevel(weightFiles, roe_path)
    CombinerLevel(weightFiles, roe_path)
    roe_path.add_module(MoveTaggerInformationToBExtraInfoModule())  # Move and remove extraInfo
    roe_path.add_module(RemoveExtraInfoModule())
    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)
