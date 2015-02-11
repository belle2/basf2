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

    """
    Detects when a ROE is empty in order to skip it. Final Extra Info is set accordingly.
    """

    def event(self):
        self.return_value(0)
        roe = Belle2.PyStoreObj('RestOfEvent')
        B0 = roe.obj().getRelated('Particles')
        if mc_variables.variables.evaluate('isRestOfEventEmpty', B0) == -2:
            B2INFO('FOUND NO TRACKS OR B0 IN ROE! NOT USED FOR TRAINING! COMBINER OUTPUT IS MANUALLY SET'
                   )
            B0.addExtraInfo('ModeCode', ModeCode)
            if ModeCode == 1:
                B0.addExtraInfo('B0Probability', 0.5)
                B0.addExtraInfo('B0barProbability', 0.5)
                B0.addExtraInfo('qrCombined', 0.0)
                B0.addExtraInfo('qrMC', -999)
                B0.addExtraInfo('NTracksInROE', 0)
                B0.addExtraInfo('NECLClustersInROE', 0)
                B0.addExtraInfo('NKLMClustersInROE', 0)
            self.return_value(1)


class RemoveExtraInfoModule(Module):

    """
    Deletes the Extrainfo saved in the used TrackLevelParticle lists.
    """

    def event(self):
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
        B0 = roe.obj().getRelated('Particles')
        B0.addExtraInfo('ModeCode', ModeCode)
        if ModeCode == 1:
            B0Probability = info.obj().getExtraInfo('qrCombined')
            B0barProbability = 1 - info.obj().getExtraInfo('qrCombined')
            qrCombined = 2 * (info.obj().getExtraInfo('qrCombined') - 0.5)
            qrMC = 2 * (mc_variables.variables.evaluate('qrCombined',
                        someParticle) - 0.5)
            NTracksInROE = roe.obj().getNTracks()
            NECLClustersInROE = roe.obj().getNECLClusters()
            NKLMClustersInROE = roe.obj().getNKLMClusters()
            B0.addExtraInfo('B0Probability', B0Probability)
            B0.addExtraInfo('B0barProbability', B0barProbability)
            B0.addExtraInfo('qrCombined', qrCombined)
            B0.addExtraInfo('qrMC', qrMC)
            B0.addExtraInfo('NTracksInROE', NTracksInROE)
            B0.addExtraInfo('NECLClustersInROE', NECLClustersInROE)
            B0.addExtraInfo('NKLMClustersInROE', NKLMClustersInROE)
            info.obj().removeExtraInfo()


# ModeCode= 0 for Teacher or =1 for Expert mode
ModeCode = float()

# Directory where the weights of the trained Methods are saved
workingDirectory = os.environ['BELLE2_LOCAL_DIR'] \
    + '/analysis/data/FlavorTagging/TrainedMethods'

# Methods for Track and Event Levels
methods = [('FastBDT', 'Plugin',
           'CreateMVAPdfs:NbinsMVAPdf=100:!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3'
           )]

# Methods for Combiner Level
methodsCombiner = [('FastBDT', 'Plugin',
                   'CreateMVAPdfs:NbinsMVAPdf=300:!H:!V:NTrees=300:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3'
                   )]

# SignalFraction: TMVA feature
# For smooth output set to -1, this will break the calibration.
# For correct calibration set to -2, leads to peaky combiner output.
signalFraction = -2

# Definition of all available categories, 'standard category name': ['ParticleList type', 'TrackLevel category name', 'EventLevel category name', 'CombinerLevel variable name', 'category code']
AvailableCategories = {
    'Electron': ['e+', 'Electron', 'Electron',
                 'QrOf(e+:ROE, IsRightClass(Electron), IsFromB(Electron))',
                 0],
    'IntermediateElectron': ['e+', 'IntermediateElectron',
                             'IntermediateElectron',
                             'QrOf(e+:ROE, IsRightClass(IntermediateElectron), IsFromB(IntermediateElectron))'
                             , 1],
    'Muon': ['mu+', 'Muon', 'Muon',
             'QrOf(mu+:ROE, IsRightClass(Muon), IsFromB(Muon))', 2],
    'IntermediateMuon': ['mu+', 'IntermediateMuon', 'IntermediateMuon',
                         'QrOf(mu+:ROE, IsRightClass(IntermediateMuon), IsFromB(IntermediateMuon))'
                         , 3],
    'KinLepton': ['mu+', 'KinLepton', 'KinLepton',
                  'QrOf(mu+:ROE, IsRightClass(KinLepton), IsFromB(KinLepton))'
                  , 4],
    'Kaon': ['K+', 'Kaon', 'Kaon',
             'InputQrOf(K+:ROE, IsRightClass(Kaon), IsFromB(Kaon))', 5],
    'SlowPion': ['pi+', 'SlowPion', 'SlowPion',
                 'QrOf(pi+:ROE, IsRightClass(SlowPion), IsFromB(SlowPion))',
                 6],
    'FastPion': ['pi+', 'FastPion', 'FastPion',
                 'QrOf(pi+:ROE, IsRightClass(FastPion), IsFromB(FastPion))',
                 7],
    'Lambda': ['Lambda0', 'Lambda', 'Lambda',
               'InputQrOf(Lambda0:ROE, IsRightClass(Lambda), IsFromB(Lambda))'
               , 8],
    'FSC': ['pi+', 'SlowPion', 'FSC',
            'QrOf(pi+:ROE, IsRightClass(FSC), IsFromB(SlowPion))', 9],
    'MaximumP*': ['pi+', 'MaximumP*', 'MaximumP*',
                  'QrOf(pi+:ROE, IsRightClass(MaximumP*), IsFromB(MaximumP*))'
                  , 10],
    'KaonPion': ['K+', 'Kaon', 'KaonPion',
                 'QrOf(K+:ROE, IsRightClass(KaonPion), IsFromB(Kaon))', 11],
    }

# Lists for each Step. TrackLevelParticles for deletion of ExtraInfo. Variables for the combiner depending of the specified categories combination.
TrackLevelParticles = []
TrackLevelParticleLists = []
EventLevelParticleLists = []
variablesCombinerLevel = []
categoriesCombinationCode = 'CatCode'


# Selection of the Categories that are going to be used.

def WhichCategories(categories=[
    'Electron',
    'Muon',
    'KinLepton',
    'Kaon',
    'SlowPion',
    'FastPion',
    'Lambda',
    'FSC',
    'MaximumP*',
    'KaonPion',
    ]):
    if len(categories) > 12 or len(categories) < 2:
        B2FATAL('Flavor Tagger: Invalid amount of categories. At least two are needed. No more than 12 are available'
                )
        B2FATAL('Flavor Tagger: Possible categories are  "Electron", "IntermediateElectron", "Muon", "IntermediateMuon", "KinLepton", "Kaon", "SlowPion", "FastPion", "Lambda", "FSC", "MaximumP*" or "KaonPion" '
                )
        return False
    categoriesCombination = []
    for category in categories:
        if category in AvailableCategories:
            if AvailableCategories[category][0] not in TrackLevelParticles:
                TrackLevelParticles.append(AvailableCategories[category][0])
            if category != 'MaximumP*' and (AvailableCategories[category][0],
                    AvailableCategories[category][1]) \
                not in TrackLevelParticleLists:
                TrackLevelParticleLists.append((AvailableCategories[category][0],
                        AvailableCategories[category][1]))
            if (AvailableCategories[category][0],
                AvailableCategories[category][2]) \
                not in EventLevelParticleLists:
                EventLevelParticleLists.append((AvailableCategories[category][0],
                        AvailableCategories[category][2]))
                variablesCombinerLevel.append(AvailableCategories[category][3])
                categoriesCombination.append(AvailableCategories[category][4])
            else:
                B2FATAL('Flavor Tagger: ' + category
                        + ' has been already given')
                return False
        else:
            B2FATAL('Flavor Tagger: ' + category
                    + ' is not a valid category name given')
            B2FATAL('Flavor Tagger: Available categories are  "Electron", "IntermediateElectron", "Muon", "IntermediateMuon", "KinLepton", "Kaon", "SlowPion", "FastPion", "Lambda", "FSC", "MaximumP*" or "KaonPion" '
                    )
            return False
    global categoriesCombinationCode
    for code in sorted(categoriesCombination):
        categoriesCombinationCode = categoriesCombinationCode + '%02d' % code
    return True


# Variables for categories on track level - are defined in variables.cc and MetaVariables.cc
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
    'chiProb',
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
    'chiProb',
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
    'chiProb',
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
    'chiProb',
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
    'chiProb',
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
# Only for Event Level
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


def TrackLevel(mode='Expert', weightFiles='B2JpsiKs_mu', path=analysis_main):
    B2INFO('TRACK LEVEL')
    ReadyMethods = 0
    for (symbol, category) in TrackLevelParticleLists:
        particleList = symbol + ':ROE'
        methodPrefixTrackLevel = weightFiles + 'TrackLevel' + category + 'TMVA'
        targetVariable = 'IsFromB(' + category + ')'

      # Select particles in ROE for different categories of flavour tagging.
        if symbol != 'Lambda0':
            fillParticleList(particleList, 'isInRestOfEvent > 0.5', path=path)
      # Check if there is K short in this event
        if symbol == 'K+':
            applyCuts('K+:ROE', '0.1<Kid', path=path)  # Precut done to prevent from overtraining, might be redundant
            fillParticleList('pi+:inKaonRoe', 'isInRestOfEvent > 0.5',
                             path=path)
            reconstructDecay('K_S0:ROEKaon -> pi+:inKaonRoe pi-:inKaonRoe',
                             '0.40<=M<=0.60', 1, path=path)
            fitVertex('K_S0:ROEKaon', 0.01, fitter='kfitter', path=path)

        if symbol == 'Lambda0':
            fillParticleList('pi+:inLambdaRoe', 'isInRestOfEvent > 0.5',
                             path=path)
            fillParticleList('p+:inLambdaRoe', 'isInRestOfEvent > 0.5',
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
            if mode == 'Expert':
                B2FATAL('Flavor Tagger: ' + symbol
                        + ' Tracklevel was not trained. Stopped')
            else:
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
                expertOutputName=targetVariable,
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


def EventLevel(mode='Expert', weightFiles='B2JpsiKs_mu', path=analysis_main):
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
            fillParticleList(particleList, 'hasHighestProbInCat('
                             + particleList + ',' + 'IsFromB(Kaon)) > 0.5',
                             path=path)
        elif category == 'FSC':
            fillParticleList(particleList, 'hasHighestProbInCat('
                             + particleList + ',' + 'IsFromB(SlowPion)) > 0.5'
                             , path=path)
        elif category == 'Lambda':
            applyCuts(particleList, 'hasHighestProbInCat(' + particleList + ','
                       + 'IsFromB(Lambda)) > 0.5', path=path)
        else:
            fillParticleList(particleList, 'hasHighestProbInCat('
                             + particleList + ',' + 'IsFromB(' + category
                             + ')) > 0.5', path=path)

        if not isTMVAMethodAvailable(workingDirectory + '/'
                                     + methodPrefixEventLevel):
            if mode == 'Expert':
                B2FATAL('Flavor Tagger: ' + symbol
                        + ' Eventlevel was not trained. Stopped')
            else:
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
                expertOutputName=targetVariable,
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


def CombinerLevel(mode='Expert', weightFiles='B2JpsiKs_mu',
                  path=analysis_main):
    B2INFO('COMBINER LEVEL')
    methodPrefixCombinerLevel = weightFiles + 'CombinerLevel' \
        + categoriesCombinationCode + 'TMVA'
    if not isTMVAMethodAvailable(workingDirectory + '/'
                                 + methodPrefixCombinerLevel):
        if mode == 'Expert':
            B2FATAL('Flavor Tagger: Combinerlevel was not trained with this combination of categories. Stopped'
                    )
        else:
            B2INFO('Train TMVAMethod on combiner level')
            trainTMVAMethod(
                [],
                variables=variablesCombinerLevel,
                target='qrCombined',
                prefix=methodPrefixCombinerLevel,
                methods=methodsCombiner,
                workingDirectory=workingDirectory,
                path=path,
                )
    else:
        B2INFO('Flavor Tagger: Ready to be used with weightFiles'
               + weightFiles + '. The training process has been finished.')
        if mode == 'Expert':
            B2INFO('Apply TMVAMethod on combiner level')
            applyTMVAMethod(
                [],
                expertOutputName='qrCombined',
                prefix=methodPrefixCombinerLevel,
                signalClass=1,
                method=methodsCombiner[0][0],
                signalFraction=-1,
                workingDirectory=workingDirectory,
                path=path,
                )
        return True


def FlavorTagger(
    mode='Expert',
    weightFiles='B2JpsiKs_mu',
    categories=[
        'Electron',
        'Muon',
        'KinLepton',
        'Kaon',
        'SlowPion',
        'FastPion',
        'Lambda',
        'FSC',
        'MaximumP*',
        'KaonPion',
        ],
    path=analysis_main,
    ):
    """
      Defines the whole flavor tagging process.
    """

    B2INFO('*** FLAVOR TAGGING ***')
    B2INFO(' ')
    B2INFO('    Working directory is: ' + workingDirectory)
    B2INFO(' ')

    global ModeCode

    if mode == 'Expert':
        ModeCode = 1
    else:
        ModeCode = 0

    roe_path = create_path()
    emptypath = create_path()

    ROEEmptyTrigger = RemoveEmptyROEModule()

    # If trigger returns 1 jump into empty path skipping further modules in roe_path
    roe_path.add_module(ROEEmptyTrigger)
    ROEEmptyTrigger.if_true(emptypath)

    # track training or expert
    if WhichCategories(categories):
        if TrackLevel(mode, weightFiles, roe_path):
            if EventLevel(mode, weightFiles, roe_path):
                CombinerLevel(mode, weightFiles, roe_path)
                roe_path.add_module(MoveTaggerInformationToBExtraInfoModule())  # Move and remove extraInfo

    roe_path.add_module(RemoveExtraInfoModule())
    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)


