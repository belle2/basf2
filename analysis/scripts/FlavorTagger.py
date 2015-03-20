#!/usr/bin/env python
# -*- coding: utf-8 -*-

# *************  Flavor Tagging   ************
# * Authors: Fernando Abudinen, Moritz Gelb  *
# *.....     and Thomas Keck
# * Contributor: Christian Roca
# * This script is needed to test            *
# * the previously trained flavor tagger.    *
# ********************************************

from basf2 import *
from modularAnalysis import *
import variables as mc_variables
from ROOT import Belle2
import os


class FlavorTaggerInfoFiller(Module):

    """
    Creates a new FlavorTagInfo DataObject and saves there all the relevant information of the
    FlavorTagging:
    - Track probability of being the right target for every category (right target means
      coming directly from the B)
    - Highest probability track's pointer
    - Event probability of belonging to a given category
    """

    def event(self):
        path = analysis_main
        info = Belle2.PyStoreObj('EventExtraInfo')  # Calls the event extra info were all Flavor Tagging Info is saved
        weightFiles = 'B2JpsiKs_mu'

        roe = Belle2.PyStoreObj('RestOfEvent')
        B0 = roe.obj().getRelated('Particles')
        FlavorTaggerInfo = roe.obj().getRelated('FlavorTagInfos')

        if not FlavorTaggerInfo:
            B2ERROR('FlavorTag does not exist')
            return

        for (particleList, category) in EventLevelParticleLists:

            # Load the Particle list in Python after the cuts
            plist = Belle2.PyStoreObj(particleList)
            if category == 'FSC' or category == 'KaonPion':
                continue

            if plist.obj().getListSize() == 0:  # From the likelihood it is possible to have Kaon category with no actual kaons
                FlavorTaggerInfo.setIsB(0)
                FlavorTaggerInfo.setTrack(None)
                FlavorTaggerInfo.setCategories(category)
                FlavorTaggerInfo.setCatProb(0)
                FlavorTaggerInfo.setTargProb(0)
                FlavorTaggerInfo.setParticle(None)

            for i in range(0, plist.obj().getListSize()):
                particle = plist.obj().getParticle(i)  # Pointer to the particle with highest prob
                track = particle.getTrack()  # Track of the particle

                if category == 'MaximumP*':  # MaximumP only gives the momentum of the Highest Momentum Particle
                    targetProb = mc_variables.variables.evaluate('p_CMS',
                                                                 particle)
                    categoryProb = 0
                else:
                    targetProb = particle.getExtraInfo('IsRightTrack(' + category
                                                       + ')')  # Prob of being the right target
                    categoryProb = particle.getExtraInfo('IsRightCategory('
                                                         + category + ')')  # Prob of belonging to a cat

    # Find Mother of MCTruth - if B (511) give True -- temporally used
                MCparticle = particle.getRelated('MCParticles')
                MCMother = MCparticle.getMother()
                if not MCMother:
                    B2ERROR(category)
                    break
                if MCMother.getPDG() == 511 or MCMother.getPDG() == -511:
                    isIt = True
                else:
                    isIt = False

    # Save information in the FlavorTagInfo DataObject
                FlavorTaggerInfo.setIsB(isIt)
                FlavorTaggerInfo.setTrack(track)
                FlavorTaggerInfo.setTargProb(targetProb)
                FlavorTaggerInfo.setParticle(particle)
                FlavorTaggerInfo.setCategories(category)
                FlavorTaggerInfo.setCatProb(categoryProb)

                break  # Temporary break that avoids saving more than 1 Lambda per event.


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
        for particleList in EventLevelParticleLists:
            plist = Belle2.PyStoreObj(particleList[0])
            for i in range(0, plist.obj().getListSize()):
                particle = plist.obj().getParticle(i)
                particle.removeExtraInfo()
        info = Belle2.PyStoreObj('EventExtraInfo')
        info.obj().removeExtraInfo()


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

# Definition of all available categories, 'standard category name':
# ['ParticleList', 'TrackLevel category name', 'EventLevel category name',
# 'CombinerLevel variable name', 'category code']
AvailableCategories = {
    'Electron': [
        'e+:ElectronROE',
        'Electron',
        'Electron',
        'QrOf(e+:ElectronROE, IsRightCategory(Electron), IsRightTrack(Electron))',
        0],
    'IntermediateElectron': [
        'e+:IntermediateElectronROE',
        'IntermediateElectron',
        'IntermediateElectron',
        'QrOf(e+:IntermediateElectronROE, IsRightCategory(IntermediateElectron), IsRightTrack(IntermediateElectron))',
        1],
    'Muon': [
        'mu+:MuonROE',
        'Muon',
        'Muon',
        'QrOf(mu+:MuonROE, IsRightCategory(Muon), IsRightTrack(Muon))',
        2],
    'IntermediateMuon': [
        'mu+:IntermediateMuonROE',
        'IntermediateMuon',
        'IntermediateMuon',
        'QrOf(mu+:IntermediateMuonROE, IsRightCategory(IntermediateMuon), IsRightTrack(IntermediateMuon))',
        3],
    'KinLepton': [
        'mu+:KinLeptonROE',
        'KinLepton',
        'KinLepton',
        'QrOf(mu+:KinLeptonROE, IsRightCategory(KinLepton), IsRightTrack(KinLepton))',
        4],
    'Kaon': [
        'K+:KaonROE',
        'Kaon',
        'Kaon',
        'InputQrOf(K+:KaonROE, IsRightCategory(Kaon), IsRightTrack(Kaon))',
        5],
    'SlowPion': [
        'pi+:SlowPionROE',
        'SlowPion',
        'SlowPion',
        'QrOf(pi+:SlowPionROE, IsRightCategory(SlowPion), IsRightTrack(SlowPion))',
        6],
    'FastPion': [
        'pi+:FastPionROE',
        'FastPion',
        'FastPion',
        'QrOf(pi+:FastPionROE, IsRightCategory(FastPion), IsRightTrack(FastPion))',
        7],
    'Lambda': [
        'Lambda0:LambdaROE',
        'Lambda',
        'Lambda',
        'InputQrOf(Lambda0:LambdaROE, IsRightCategory(Lambda), IsRightTrack(Lambda))',
        8],
    'FSC': [
        'pi+:SlowPionROE',
        'SlowPion',
        'FSC',
        'QrOf(pi+:SlowPionROE, IsRightCategory(FSC), IsRightTrack(SlowPion))',
        9],
    'MaximumP*': [
        'pi+:MaximumP*ROE',
        'MaximumP*',
        'MaximumP*',
        'QrOf(pi+:MaximumP*ROE, IsRightCategory(MaximumP*), IsRightTrack(MaximumP*))',
        10],
    'KaonPion': [
        'K+:KaonROE',
        'Kaon',
        'KaonPion',
        'QrOf(K+:KaonROE, IsRightCategory(KaonPion), IsRightTrack(Kaon))',
        11],
}

# Lists for each Step.
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
        B2FATAL(
            'Flavor Tagger: Possible categories are  "Electron", "IntermediateElectron", "Muon", "IntermediateMuon", '
            '"KinLepton", "Kaon", "SlowPion", "FastPion", "Lambda", "FSC", "MaximumP*" or "KaonPion" ')
        return False
    categoriesCombination = []
    for category in categories:
        if category in AvailableCategories:
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
            B2FATAL(
                'Flavor Tagger: Available categories are  "Electron", "IntermediateElectron", "Muon", "IntermediateMuon", '
                '"KinLepton", "Kaon", "SlowPion", "FastPion", "Lambda", "FSC", "MaximumP*" or "KaonPion" ')
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
variables['KaonPion'] = ['HighestProbInCat(K+:KaonROE, IsRightTrack(Kaon))',
                         'HighestProbInCat(pi+:SlowPionROE, IsRightTrack(SlowPion))',
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
    for (particleList, category) in TrackLevelParticleLists:
        methodPrefixTrackLevel = weightFiles + 'TrackLevel' + category + 'TMVA'
        targetVariable = 'IsRightTrack(' + category + ')'

        # Select particles in ROE for different categories of flavour tagging.
        if particleList != 'Lambda0:LambdaROE':
            fillParticleList(particleList, 'isInRestOfEvent > 0.5', path=path)
        # Check if there is K short in this event
        if particleList == 'K+:KaonROE':
            applyCuts(particleList, '0.1<Kid', path=path)  # Precut done to prevent from overtraining, might be redundant
            fillParticleList('pi+:inKaonRoe', 'isInRestOfEvent > 0.5',
                             path=path)
            reconstructDecay('K_S0:ROEKaon -> pi+:inKaonRoe pi-:inKaonRoe',
                             '0.40<=M<=0.60', 1, path=path)
            fitVertex('K_S0:ROEKaon', 0.01, fitter='kfitter', path=path)

        if particleList == 'Lambda0:LambdaROE':
            fillParticleList('pi+:inLambdaRoe', 'isInRestOfEvent > 0.5',
                             path=path)
            fillParticleList('p+:inLambdaRoe', 'isInRestOfEvent > 0.5',
                             path=path)
            reconstructDecay(particleList + ' -> pi-:inLambdaRoe p+:inLambdaRoe',
                             '1.00<=M<=1.23', 1, path=path)
            reconstructDecay('K_S0:ROELambda -> pi+:inLambdaRoe pi-:inLambdaRoe', '0.40<=M<=0.60', 1, path=path)
            fitVertex(particleList, 0.01, fitter='kfitter', path=path)
            matchMCTruth(particleList, path=path)
            fitVertex('K_S0:ROELambda', 0.01, fitter='kfitter', path=path)

        if not isTMVAMethodAvailable(workingDirectory + '/'
                                     + methodPrefixTrackLevel):
            if mode == 'Expert':
                B2FATAL('Flavor Tagger: ' + particleList
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
    for (particleList, category) in EventLevelParticleLists:
        methodPrefixEventLevel = weightFiles + 'EventLevel' + category + 'TMVA'
        targetVariable = 'IsRightCategory(' + category + ')'
        if category == 'MaximumP*':
            fillParticleList(particleList, 'isInRestOfEvent > 0.5', path=path)

        if not isTMVAMethodAvailable(workingDirectory + '/'
                                     + methodPrefixEventLevel):
            if mode == 'Expert':
                B2FATAL('Flavor Tagger: ' + particleList
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
            # if category == 'KinLepton':
                # applyCuts(particleList, 'isInElectronOrMuonCat < 0.5', path=path)
            if category == 'KaonPion':
                applyCuts(particleList, 'hasHighestProbInCat(' + particleList + ','
                          + 'IsRightTrack(Kaon)) > 0.5', path=path)
            elif category == 'FSC':
                applyCuts(particleList, 'hasHighestProbInCat(' + particleList + ','
                          + 'IsRightTrack(SlowPion)) > 0.5', path=path)
            elif category == 'Lambda':
                applyCuts(particleList, 'hasHighestProbInCat(' + particleList + ','
                          + 'IsRightTrack(Lambda)) > 0.5', path=path)
            else:
                applyCuts(particleList, 'hasHighestProbInCat(' + particleList + ','
                          + 'IsRightTrack(' + category + ')) > 0.5', path=path)

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
    recoParticle,
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

    roe_path.add_module(MoveTaggerInformationToBExtraInfoModule())  # Move EventExtraInfo to ParticleExtraInfo

    if mode == 'Expert':
        # Initialation of FlavorTagInfo dataObject needs to be done in the main path
        FlavorTagInfoBuilder = register_module('FlavorTagInfoBuilder')
        FlavorTagInfoBuilder.param('particleList', recoParticle)
        path.add_module(FlavorTagInfoBuilder)

        roe_path.add_module(FlavorTaggerInfoFiller())  # Add FlavorTag Info filler to roe_path
        roe_path.add_module(RemoveExtraInfoModule())  # Removes EventExtraInfos and ParticleExtraInfos of the EventParticleLists

    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)
