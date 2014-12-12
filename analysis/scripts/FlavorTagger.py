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

# Variables for all Categories (Track and Event Levels)
variables = dict()
variables['Electron'] = [
    'p_CMS',
    'pt_CMS',
    'p',
    'pt',
    'eid',
    'eid_dEdx',
    'eid_TOP',
    'SemiLeptonicVariables(recoilMass)',
    'SemiLeptonicVariables(p_missing_CMS)',
    'SemiLeptonicVariables(CosTheta_missing_CMS)',
    'SemiLeptonicVariables(EW90)',
    'eid_ARICH',
    'eid_ECL',
    ]
variables['IntermediateElectron'] = variables['Electron']
variables['Muon'] = [
    'p_CMS',
    'pt_CMS',
    'p',
    'pt',
    'muid',
    'muid_dEdx',
    'muid_TOP',
    'SemiLeptonicVariables(recoilMass)',
    'SemiLeptonicVariables(p_missing_CMS)',
    'SemiLeptonicVariables(CosTheta_missing_CMS)',
    'SemiLeptonicVariables(EW90)',
    'muid_ARICH',
    ]
variables['IntermediateMuon'] = variables['Muon']
variables['KinLepton'] = [
    'p_CMS',
    'pt_CMS',
    'p',
    'pt',
    'muid',
    'muid_dEdx',
    'muid_TOP',
    'SemiLeptonicVariables(recoilMass)',
    'SemiLeptonicVariables(p_missing_CMS)',
    'SemiLeptonicVariables(CosTheta_missing_CMS)',
    'SemiLeptonicVariables(EW90)',
    'muid_ARICH',
    'eid',
    'eid_dEdx',
    'eid_TOP',
    'eid_ARICH',
    'eid_ECL',
    ]
variables['Kaon'] = [
    'p_CMS',
    'pt_CMS',
    'cosTheta',
    'pt',
    'Kid',
    'Kid_dEdx',
    'Kid_TOP',
    'Kid_ARICH',
    'NumberOfKShortinRemainingROEKaon',
    'ptTracksRoe',
    'distance',
    ]
variables['SlowPion'] = [
    'p_CMS',
    'pt_CMS',
    'cosTheta',
    'p',
    'pt',
    'piid',
    'piid_dEdx',
    'piid_TOP',
    'piid_ARICH',
    'pi_vs_edEdxid',
    'cosTPTO',
    'Kid',
    'eid',
    ]
variables['FastPion'] = variables['SlowPion']
variables['Lambda'] = [
    'lambdaFlavor',
    'NumberOfKShortinRemainingROELambda',
    'M',
    'cosAngleBetweenMomentumAndVertexVector',
    'lambdaZError',
    'MomentumOfSecondDaughter',
    'MomentumOfSecondDaughter_CMS',
    'p_CMS',
    'p',
    'chiProb',
    'distance',
    ]
variables['KaonPion'] = ['HighestProbInCat(K+:ROE, IsFromB(Kaon))',
                         'HighestProbInCat(pi+:ROE, IsFromB(SlowPion))',
                         'cosKaonPion', 'KaonPionHaveOpositeCharges', 'Kid']
variables['MaximumP*'] = [
    'p_CMS',
    'pt_CMS',
    'p',
    'pt',
    'cosTPTO',
    'ImpactXY',
    ]
variables['FSC'] = [
    'p_CMS',
    'cosTPTO',
    'Kid',
    'FSCVariables(p_CMS_Fast)',
    'FSCVariables(cosSlowFast)',
    'FSCVariables(cosTPTO_Fast)',
    'FSCVariables(SlowFastHaveOpositeCharges)',
    ]

# Variables for the Combiner
variablesCombiner = [  # 'QrOf(e+:ROE, IsRightClass(IntermediateElectron), IsFromB(IntermediateElectron))'
                       # ,
                       # 'QrOf(mu+:ROE, IsRightClass(IntermediateMuon), IsFromB(IntermediateMuon))'
                       # ,
    'QrOf(e+:ROE, IsRightClass(Electron), IsFromB(Electron))',
    'QrOf(mu+:ROE, IsRightClass(Muon), IsFromB(Muon))',
    'QrOf(mu+:ROE, IsRightClass(KinLepton), IsFromB(KinLepton))',
    'QrOf(K+:ROE, IsRightClass(Kaon), IsFromB(Kaon))',
    'QrOf(pi+:ROE, IsRightClass(SlowPion), IsFromB(SlowPion))',
    'QrOf(pi+:ROE, IsRightClass(FastPion), IsFromB(FastPion))',
    'QrOf(pi+:ROE, IsRightClass(MaximumP*), IsFromB(MaximumP*))',
    'QrOf(pi+:ROE, IsRightClass(FSC), IsFromB(SlowPion))',
    'QrOf(K+:ROE, IsRightClass(KaonPion), IsFromB(Kaon))',
    'QrOf(Lambda0:ROE, IsRightClass(Lambda), IsFromB(Lambda))',
    ]

# Methods for Track and Event Levels
methods = [('FastBDT', 'Plugin',
           'CreateMVAPdfs:NbinsMVAPdf=100:!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3'
           )]

# Methods for Combiner Level
methodsCombiner = [('FastBDT', 'Plugin',
                   'CreateMVAPdfs:NbinsMVAPdf=300:!H:!V:NTrees=300:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3'
                   )]
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
    ('Lambda0', 'Lambda'),
    ]

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
    ('Lambda0', 'Lambda'),
    ]


def TrackLevel(weightFiles='B2JpsiKs_mu', path=analysis_main):
    B2INFO('TRACK LEVEL')
    ReadyMethods = 0
    for (symbol, category) in TrackLevelParticleLists:
        particleList = symbol + ':ROE'
        methodPrefixTrackLevel = weightFiles + 'TrackLevel' + category + 'TMVA'
        targetVariable = 'IsFromB(' + category + ')'

      # Select particles in ROE for different categories of flavour tagging.
        if symbol != 'Lambda0':
            selectParticle(particleList,
                           'isInRestOfEvent > 0.5 and chiProb > 0.001',
                           path=path)
      # Check if there is K short in this event
        if symbol == 'K+':
            applyCuts('K+:ROE', '0.1<Kid', path=path)  # Precut done to prevent from overtraining, might be redundant
            selectParticle('pi+:inKaonRoe',
                           'isInRestOfEvent > 0.5 and chiProb > 0.001',
                           path=path)
            reconstructDecay('K_S0:ROEKaon -> pi+:inKaonRoe pi-:inKaonRoe',
                             '0.40<=M<=0.60', 1, path=path)
            fitVertex('K_S0:ROEKaon', 0.01, fitter='kfitter', path=path)

        if symbol == 'Lambda0':
            selectParticle('pi+:inLambdaRoe',
                           'isInRestOfEvent > 0.5 and chiProb > 0.001',
                           path=path)
            selectParticle('p+:inLambdaRoe',
                           'isInRestOfEvent > 0.5 and chiProb > 0.001',
                           path=path)
            reconstructDecay('Lambda0:ROE -> pi-:inLambdaRoe p+:inLambdaRoe',
                             '1.00<=M<=1.23', 1, path=path)
            reconstructDecay('K_S0:ROELambda -> pi+:inLambdaRoe pi-:inLambdaRoe'
                             , '0.40<=M<=0.60', 1, path=path)
            fitVertex('Lambda0:ROE', 0.01, fitter='kfitter', path=path)
            matchMCTruth('Lambda0:ROE', path=path)
            fitVertex('K_S0:ROELambda', 0.01, fitter='kfitter', path=path)

        if not isTMVAMethodAvailable(workingDirectory + '/'
                                     + methodPrefixTrackLevel):
            B2INFO('PROCESSING: trainTMVAMethod on track level')
            trainTMVAMethod(
                particleList,
                variables=variables[category],
                target=targetVariable,
                methods=methods,
                prefix=methodPrefixTrackLevel,
                workingDirectory=workingDirectory,
                path=path,
                )
        else:
            B2INFO('PROCESSING: applyTMVAMethod on track level')
            applyTMVAMethod(
                particleList,
                prefix=methodPrefixTrackLevel,
                signalProbabilityName=targetVariable,
                method=methods[0][0],
                signalFraction=signalFraction,
                workingDirectory=workingDirectory,
                path=path,
                )
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
            selectParticle(particleList, 'hasHighestProbInCat(' + particleList
                           + ',' + 'IsFromB(Kaon)) > 0.5', path=path)
        elif category == 'FSC':
            selectParticle(particleList, 'hasHighestProbInCat(' + particleList
                           + ',' + 'IsFromB(SlowPion)) > 0.5', path=path)
        else:
            selectParticle(particleList, 'hasHighestProbInCat(' + particleList
                           + ',' + 'IsFromB(' + category + ')) > 0.5',
                           path=path)

        if not isTMVAMethodAvailable(workingDirectory + '/'
                                     + methodPrefixEventLevel):
            B2INFO('PROCESSING: trainTMVAMethod on event level')
            trainTMVAMethod(
                particleList,
                variables=variables[category],
                target=targetVariable,
                prefix=methodPrefixEventLevel,
                methods=methods,
                workingDirectory=workingDirectory,
                path=path,
                )
        else:
            B2INFO('PROCESSING: applyTMVAMethod on event level')
            applyTMVAMethod(
                particleList,
                prefix=methodPrefixEventLevel,
                signalProbabilityName=targetVariable,
                method=methods[0][0],
                signalFraction=signalFraction,
                workingDirectory=workingDirectory,
                path=path,
                )
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
        B2INFO('Train TMVAMethod on combiner level')
        trainTMVAMethod(
            [],
            variables=variablesCombiner,
            target='qrCombined',
            prefix=methodPrefixCombinerLevel,
            methods=methodsCombiner,
            workingDirectory=workingDirectory,
            path=path,
            )
        return False
    else:
        B2INFO('Apply TMVAMethod on combiner level')
        applyTMVAMethod(
            [],
            signalProbabilityName='qrCombined',
            prefix=methodPrefixCombinerLevel,
            signalClass=1,
            method=methodsCombiner[0][0],
            signalFraction=signalFraction,
            workingDirectory=workingDirectory,
            path=path,
            )
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

    TrackLevel(weightFiles, roe_path)

    if TrackLevel(weightFiles, roe_path):
        EventLevel(weightFiles, roe_path)

    if EventLevel(weightFiles, roe_path):
        CombinerLevel(weightFiles, roe_path)
    else:
        roe_path.add_module(RemoveExtraInfoModule())

    if CombinerLevel(weightFiles, roe_path):
        roe_path.add_module(MoveTaggerInformationToBExtraInfoModule())  # Move and remove extraInfo
        roe_path.add_module(RemoveExtraInfoModule())

    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)


