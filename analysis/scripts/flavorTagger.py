#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# *************  Flavor Tagging   ************
# * Authors: Fernando Abudinen, Moritz Gelb  *
# *.....     and Thomas Keck
# * Contributor: Christian Roca
# * This script is needed to train           *
# * and to test the flavor tagger.           *
# ********************************************

from basf2 import *
from modularAnalysis import *
import basf2_mva
import variables as mc_variables
from ROOT import Belle2
import os


class flavorTaggerInfoFiller(Module):

    """
    Creates a new flavorTaggerInfoMap DataObject for a specific method. Saves there all the relevant information of the
    flavorTagger:
    -The pointer to the track with highest target probability in Track Level for each category*/
    -The highest target track probability in Track Level for each category*/
    -The pointer to the track with highest category probability in Event Level*/
    -The highest category probability in Event Level for each category*/
    -The qr Output of each category, i.e. the Combiner input values. They could be used for independent tags.*/

    -qr Output of the Combiner.
    -Direct Output of the Combiner: Probability of being a B0.*/
    -Direct Output of the Combiner: Probability of being a B0bar).*/
    """

    def event(self):
        """ Process for each event """

        path = analysis_main
        info = Belle2.PyStoreObj('EventExtraInfo')  # Calls the event extra info were all Flavor Tagging Info is saved
        weightFiles = 'B2JpsiKs_mu'

        roe = Belle2.PyStoreObj('RestOfEvent')
        flavorTaggerInfo = roe.obj().getRelated('FlavorTaggerInfos')

        flavorTaggerInfo.setUseModeFlavorTagger("Expert")

        infoMaps = dict()

        if TMVAfbdt:
            flavorTaggerInfo.addMethodMap("TMVA")
            infoMaps["TMVA"] = flavorTaggerInfo.getMethodMap("TMVA")

            qrCombined = 2 * (info.obj().getExtraInfo('qrCombinedTMVA') - 0.5)
            B0Probability = info.obj().getExtraInfo('qrCombinedTMVA')
            B0barProbability = 1 - info.obj().getExtraInfo('qrCombinedTMVA')
            infoMaps["TMVA"].setQrCombined(qrCombined)
            infoMaps["TMVA"].setB0Probability(B0Probability)
            infoMaps["TMVA"].setB0barProbability(B0barProbability)

        if FANNmlp:
            flavorTaggerInfo.addMethodMap("FANN")
            infoMaps["FANN"] = flavorTaggerInfo.getMethodMap("FANN")

            qrCombined = 2 * (info.obj().getExtraInfo('qrCombinedFANN') - 0.5)
            B0Probability = info.obj().getExtraInfo('qrCombinedFANN')
            B0barProbability = 1 - info.obj().getExtraInfo('qrCombinedFANN')
            infoMaps["FANN"].setQrCombined(qrCombined)
            infoMaps["FANN"].setB0Probability(B0Probability)
            infoMaps["FANN"].setB0barProbability(B0barProbability)

        if not flavorTaggerInfo:
            B2ERROR('flavorTaggerInfoFiller: FlavorTaggerInfo does not exist')
            return

        for (particleList, category) in trackLevelParticleLists:
            # Load the Particle list in Python after the cuts in Track Level
            plist = Belle2.PyStoreObj(particleList)

            # From the likelihood it is possible to have Kaon category with no actual kaons
            if plist.obj().getListSize() == 0:
                infoMaps["TMVA"].setTargetTrackLevel(category, None)
                infoMaps["TMVA"].setProbTrackLevel(category, 0)

            for i in range(0, plist.obj().getListSize()):
                particle = plist.obj().getParticle(i)  # Pointer to the particle with highest prob
                if mc_variables.variables.evaluate(
                        'hasHighestProbInCat(' + particleList + ',' + 'isRightTrack(' + category + '))',
                        particle) == 1:
                    # Prob of being the right target track
                    targetProb = particle.getExtraInfo('isRightTrack(' + category + ')')
                    track = particle.getTrack()  # Track of the particle
                    infoMaps["TMVA"].setTargetTrackLevel(category, track)
                    infoMaps["TMVA"].setProbTrackLevel(category, targetProb)
                    break

        for (particleList, category) in eventLevelParticleLists:
            # Load the Particle list in Python after the cuts in Event Level
            plist = Belle2.PyStoreObj(particleList)

            # From the likelihood it is possible to have Kaon category with no actual kaons
            if plist.obj().getListSize() == 0:
                infoMaps["TMVA"].setTargetEventLevel(category, None)
                infoMaps["TMVA"].setProbEventLevel(category, 0)
                infoMaps["TMVA"].setQrCategory(category, 0)

            for i in range(0, plist.obj().getListSize()):
                particle = plist.obj().getParticle(i)  # Pointer to the particle with highest prob
                if mc_variables.variables.evaluate(
                        'hasHighestProbInCat(' + particleList + ',' + 'isRightCategory(' + category + '))',
                        particle) == 1:
                    # Prob of belonging to a cat
                    categoryProb = particle.getExtraInfo('isRightCategory(' + category + ')')
                    track = particle.getTrack()  # Track of the particle
                    qrCategory = mc_variables.variables.evaluate(AvailableCategories[category][3], particle)

                    # Save information in the FlavorTaggerInfo DataObject
                    infoMaps["TMVA"].setTargetEventLevel(category, track)
                    infoMaps["TMVA"].setProbEventLevel(category, categoryProb)
                    infoMaps["TMVA"].setQrCategory(category, qrCategory)
                    break


class RemoveEmptyROEModule(Module):

    """
    Detects when a ROE does not contain tracks in order to skip it.
    """

    def event(self):
        """ Process for each event """
        self.return_value(0)
        roe = Belle2.PyStoreObj('RestOfEvent')
        B0 = roe.obj().getRelated('Particles')
        if mc_variables.variables.evaluate('hasRestOfEventTracks', B0) == 0:
            B2INFO('flavorTagger: FOUND NO TRACKS IN ROE! COMBINER OUTPUT IS THE DEFAULT -2.'
                   )
            self.return_value(1)


class RemoveWrongMCMatchedROEs(Module):

    """
    Detects when a ROE corresponds to a wrongly reconstructed event although it is not empty.
    This is done checking the MCerror of Breco.
    """

    def event(self):
        """ Process for each event """
        self.return_value(0)
        someParticle = Belle2.Particle(None)
        if mc_variables.variables.evaluate('qrCombined', someParticle) < 0:
            B2INFO('flavorTagger: FOUND NO B-MESON IN ROE! EVENT WILL BE DISCARDED FOR TRAINING!'
                   )
            self.return_value(1)


class RemoveExtraInfoModule(Module):

    """
    Deletes the Extrainfo saved in the used trackLevelParticle lists.
    """

    def event(self):
        """ Process for each event """
        ModeCode = getModeCode()
        for particleList in eventLevelParticleLists:
            plist = Belle2.PyStoreObj(particleList[0])
            for i in range(0, plist.obj().getListSize()):
                particle = plist.obj().getParticle(i)
                particle.removeExtraInfo()
        if ModeCode == 1:
            info = Belle2.PyStoreObj('EventExtraInfo')
            info.obj().removeExtraInfo()


def setModeCode(mode='Expert'):
    """
    Sets ModeCode= 0 for Teacher or =1 for Expert mode.
    """

    global ModeCode
    if mode == 'Expert':
        ModeCode = 1
    else:
        ModeCode = 0


def getModeCode():
    """
    Gets the global ModeCode.
    """

    if ModeCode == 1:
        return 1
    else:
        return 0


def setBelleOrBelle2(belleOrBelle2='Belle2'):
    """
    Sets belleOrBelle2Flag according to the specified argument.
    """

    global belleOrBelle2Flag
    belleOrBelle2Flag = belleOrBelle2


def getBelleOrBelle2():
    """
    Gets the global ModeCode.
    """
    return belleOrBelle2Flag


# Methods for Track and Event Levels
fastBDTCategories = basf2_mva.FastBDTOptions()
fastBDTCategories.m_nTrees = 400  # Before 100
fastBDTCategories.m_nCuts = 8
fastBDTCategories.m_nLevels = 3
fastBDTCategories.m_shrinkage = 0.10
fastBDTCategories.m_randRatio = 0.5

fastBDTCombiner = basf2_mva.FastBDTOptions()
fastBDTCombiner.m_nTrees = 400  # Before 300
fastBDTCombiner.m_nCuts = 8
fastBDTCombiner.m_nLevels = 3
fastBDTCombiner.m_shrinkage = 0.10
fastBDTCombiner.m_randRatio = 0.5


methods = [
    ('FastBDT', 'Plugin', 'CreateMVAPdfs:NbinsMVAPdf=100:!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3')
]

# Methods for Combiner Level
methodsCombinerTMVA = [
    ('FastBDT', 'Plugin', 'CreateMVAPdfs:NbinsMVAPdf=300:!H:!V:NTrees=300:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3')
]
methodsCombinerFANN = [('MLP', 'FANN', 'NCycles=10000:HiddenLayers=3*N:NeuronType=FANN_SIGMOID_SYMMETRIC:'
                        'ValidationFraction=0.5:RandomSeeds=10:TrainingMethod=FANN_TRAIN_RPROP:TestRate=500:'
                        'NThreads=8:EpochMonitoring=True')]

# SignalFraction: TMVA feature
# For smooth output set to -1, this will break the calibration.
# For correct calibration set to -2, leads to peaky combiner output.
signalFraction = -2

# Definition of all available categories, 'standard category name':
# ['ParticleList', 'trackLevel category name', 'eventLevel category name',
# 'combinerLevel variable name', 'category code']
AvailableCategories = {
    'Electron': [
        'e+:ElectronROE',
        'Electron',
        'Electron',
        'QrOf(e+:ElectronROE, isRightCategory(Electron), isRightTrack(Electron))',
        0],
    'IntermediateElectron': [
        'e+:IntermediateElectronROE',
        'IntermediateElectron',
        'IntermediateElectron',
        'QrOf(e+:IntermediateElectronROE, isRightCategory(IntermediateElectron), isRightTrack(IntermediateElectron))',
        1],
    'Muon': [
        'mu+:MuonROE',
        'Muon',
        'Muon',
        'QrOf(mu+:MuonROE, isRightCategory(Muon), isRightTrack(Muon))',
        2],
    'IntermediateMuon': [
        'mu+:IntermediateMuonROE',
        'IntermediateMuon',
        'IntermediateMuon',
        'QrOf(mu+:IntermediateMuonROE, isRightCategory(IntermediateMuon), isRightTrack(IntermediateMuon))',
        3],
    'KinLepton': [
        'mu+:KinLeptonROE',
        'KinLepton',
        'KinLepton',
        'QrOf(mu+:KinLeptonROE, isRightCategory(KinLepton), isRightTrack(KinLepton))',
        4],
    'IntermediateKinLepton': [
        'mu+:IntermediateKinLeptonROE',
        'IntermediateKinLepton',
        'IntermediateKinLepton',
        'QrOf(mu+:IntermediateKinLeptonROE, isRightCategory(IntermediateKinLepton), isRightTrack(IntermediateKinLepton))',
        5],
    'Kaon': [
        'K+:KaonROE',
        'Kaon',
        'Kaon',
        'weightedQrOf(K+:KaonROE, isRightCategory(Kaon), isRightTrack(Kaon))',
        6],
    'SlowPion': [
        'pi+:SlowPionROE',
        'SlowPion',
        'SlowPion',
        'QrOf(pi+:SlowPionROE, isRightCategory(SlowPion), isRightTrack(SlowPion))',
        7],
    'FastPion': [
        'pi+:FastPionROE',
        'FastPion',
        'FastPion',
        'QrOf(pi+:FastPionROE, isRightCategory(FastPion), isRightTrack(FastPion))',
        8],
    'Lambda': [
        'Lambda0:LambdaROE',
        'Lambda',
        'Lambda',
        'weightedQrOf(Lambda0:LambdaROE, isRightCategory(Lambda), isRightTrack(Lambda))',
        9],
    'FSC': [
        'pi+:SlowPionROE',
        'SlowPion',
        'FSC',
        'QrOf(pi+:SlowPionROE, isRightCategory(FSC), isRightTrack(SlowPion))',
        10],
    'MaximumPstar': [
        'pi+:MaximumPstarROE',
        'MaximumPstar',
        'MaximumPstar',
        'QrOf(pi+:MaximumPstarROE, isRightCategory(MaximumPstar), isRightTrack(MaximumPstar))',
        11],
    'KaonPion': [
        'K+:KaonROE',
        'Kaon',
        'KaonPion',
        'QrOf(K+:KaonROE, isRightCategory(KaonPion), isRightTrack(Kaon))',
        12],
}

# Lists for each Step.
trackLevelParticleLists = []
eventLevelParticleLists = []
variablesCombinerLevel = []
categoriesCombinationCode = 'CatCode'


def WhichCategories(categories=[
    'Electron',
    'Muon',
    'KinLepton',
    'Kaon',
    'SlowPion',
    'FastPion',
    'Lambda',
    'FSC',
    'MaximumPstar',
    'KaonPion',
]):
    """
    Selection of the Categories that are going to be used.
    """

    if len(categories) > 13 or len(categories) < 2:
        B2FATAL('Flavor Tagger: Invalid amount of categories. At least two are needed. No more than 13 are available'
                )
        B2FATAL(
            'Flavor Tagger: Possible categories are  "Electron", "IntermediateElectron", "Muon", "IntermediateMuon", '
            '"KinLepton", "IntermediateKinLepton", "Kaon", "SlowPion", "FastPion", "Lambda", "FSC", "MaximumPstar" or "KaonPion" ')
        return False
    categoriesCombination = []
    for category in categories:
        if category in AvailableCategories:
            if category != 'MaximumPstar' and (AvailableCategories[category][0],
                                               AvailableCategories[category][1]) \
                    not in trackLevelParticleLists:
                trackLevelParticleLists.append((AvailableCategories[category][0],
                                                AvailableCategories[category][1]))
            if (AvailableCategories[category][0],
                    AvailableCategories[category][2]) \
                    not in eventLevelParticleLists:
                eventLevelParticleLists.append((AvailableCategories[category][0],
                                                AvailableCategories[category][2]))
                variablesCombinerLevel.append(AvailableCategories[category][3])
                categoriesCombination.append(AvailableCategories[category][4])
            else:
                B2FATAL('Flavor Tagger: ' + category + ' has been already given')
                return False
        else:
            B2FATAL('Flavor Tagger: ' + category + ' is not a valid category name given')
            B2FATAL('Flavor Tagger: Available categories are  "Electron", "IntermediateElectron", '
                    '"Muon", "IntermediateMuon", "KinLepton", "IntermediateKinLepton", "Kaon", "SlowPion", "FastPion", '
                    '"Lambda", "FSC", "MaximumPstar" or "KaonPion" ')
            return False
    global categoriesCombinationCode
    for code in sorted(categoriesCombination):
        categoriesCombinationCode = categoriesCombinationCode + '%02d' % code
    return True


# Variables for categories on track level - are defined in variables.cc and MetaVariables.cc
variables = dict()
KId = {'Belle': 'Kid_belle', 'Belle2': 'Kid'}
muId = {'Belle': 'muIDBelle', 'Belle2': 'muid'}


def setVariables():
    """
    Sets the Variables used for Track and Event Levels.
    """

    variables['Electron'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'p',
        'pt',
        'eid',
        'eid_dEdx',
        'eid_TOP',
        'SemiLeptonicVariables(recoilMass)',
        'SemiLeptonicVariables(pMissCMS)',
        'SemiLeptonicVariables(cosThetaMissCMS)',
        'SemiLeptonicVariables(EW90)',
        'eid_ARICH',
        'eid_ECL',
        'chiProb',
    ]
    variables['IntermediateElectron'] = variables['Electron']
    variables['Muon'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'p',
        'pt',
        muId[getBelleOrBelle2()],
        'muid_dEdx',
        'muid_TOP',
        'SemiLeptonicVariables(recoilMass)',
        'SemiLeptonicVariables(pMissCMS)',
        'SemiLeptonicVariables(cosThetaMissCMS)',
        'SemiLeptonicVariables(EW90)',
        'muid_ARICH',
        'chiProb',
    ]
    variables['IntermediateMuon'] = variables['Muon']
    variables['KinLepton'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'p',
        'pt',
        muId[getBelleOrBelle2()],
        'muid_dEdx',
        'muid_TOP',
        'SemiLeptonicVariables(recoilMass)',
        'SemiLeptonicVariables(pMissCMS)',
        'SemiLeptonicVariables(cosThetaMissCMS)',
        'SemiLeptonicVariables(EW90)',
        'muid_ARICH',
        'eid',
        'eid_dEdx',
        'eid_TOP',
        'eid_ARICH',
        'eid_ECL',
        'chiProb',
    ]
    variables['IntermediateKinLepton'] = variables['KinLepton']
    variables['Kaon'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'cosTheta',
        'pt',
        KId[getBelleOrBelle2()],
        'Kid_dEdx',
        'Kid_TOP',
        'Kid_ARICH',
        'NumberOfKShortinROEParticleList(K_S0:ROEKaon)',
        'ptTracksRoe',
        'distance',
        'chiProb',
    ]
    variables['SlowPion'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'cosTheta',
        'p',
        'pt',
        'piid',
        'piid_dEdx',
        'piid_TOP',
        'piid_ARICH',
        'pi_vs_edEdxid',
        'cosTPTO',
        KId[getBelleOrBelle2()],
        'eid',
        'chiProb',
    ]
    variables['FastPion'] = variables['SlowPion']
    variables['Lambda'] = [
        'lambdaFlavor',
        'NumberOfKShortinROEParticleList(K_S0:ROELambda)',
        'M',
        'cosAngleBetweenMomentumAndVertexVector',
        'lambdaZError',
        'daughter(1,p)',
        'daughter(1,useCMSFrame(p))',
        'useCMSFrame(p)',
        'p',
        'chiProb',
        'distance',
    ]

    # Only for Event Level
    variables['KaonPion'] = ['HighestProbInCat(K+:KaonROE, isRightTrack(Kaon))',
                             'HighestProbInCat(pi+:SlowPionROE, isRightTrack(SlowPion))',
                             'KaonPionVariables(cosKaonPion)', 'KaonPionVariables(HaveOpositeCharges)', 'Kid']

    variables['MaximumPstar'] = [
        'useCMSFrame(p)',
        'useCMSFrame(pt)',
        'p',
        'pt',
        'cosTPTO',
        'ImpactXY',
    ]

    variables['FSC'] = [
        'useCMSFrame(p)',
        'cosTPTO',
        'Kid',
        'FSCVariables(pFastCMS)',
        'FSCVariables(cosSlowFast)',
        'FSCVariables(cosTPTOFast)',
        'FSCVariables(SlowFastHaveOpositeCharges)',
    ]


def FillParticleLists(mode='Expert', path=analysis_main):
    """
    Fills the particle Lists for each category.
    """

    for (particleList, category) in trackLevelParticleLists:

        # Select particles in ROE for different categories of flavour tagging.
        if particleList != 'Lambda0:LambdaROE':

            # Filling particle list for actual category
            fillParticleList(particleList, 'isInRestOfEvent > 0.5 and isNAN(p) !=1 and isInfinity(p) != 1', path=path)

        # Check if there is K short in this event
        if particleList == 'K+:KaonROE':
            # Precut done to prevent from overtraining, might be redundant
            applyCuts(particleList, '0.1<' + KId[getBelleOrBelle2()], path=path)
            fillParticleList('pi+:inKaonRoe', 'isInRestOfEvent > 0.5 and isNAN(p) !=1 and isInfinity(p) != 1',
                             path=path)
            reconstructDecay('K_S0:ROEKaon -> pi+:inKaonRoe pi-:inKaonRoe',
                             '0.40<=M<=0.60', True, path=path)
            fitVertex('K_S0:ROEKaon', 0.01, fitter='kfitter', path=path)

        if particleList == 'Lambda0:LambdaROE':
            fillParticleList('pi+:inLambdaRoe', 'isInRestOfEvent > 0.5 and isNAN(p) !=1 and isInfinity(p) != 1',
                             path=path)
            fillParticleList('p+:inLambdaRoe', 'isInRestOfEvent > 0.5 and isNAN(p) !=1 and isInfinity(p) != 1',
                             path=path)
            reconstructDecay(particleList + ' -> pi-:inLambdaRoe p+:inLambdaRoe',
                             '1.00<=M<=1.23', True, path=path)
            reconstructDecay('K_S0:ROELambda -> pi+:inLambdaRoe pi-:inLambdaRoe', '0.40<=M<=0.60', True, path=path)
            fitVertex(particleList, 0.01, fitter='kfitter', path=path)
            if mode != 'Expert':
                matchMCTruth(particleList, path=path)
            fitVertex('K_S0:ROELambda', 0.01, fitter='kfitter', path=path)

    # Filling 'pi+:MaximumPstarROE' particle list
    fillParticleList('pi+:MaximumPstarROE', 'isInRestOfEvent > 0.5 and isNAN(p) !=1 and isInfinity(p) != 1', path=path)

    return True


def trackLevel(mode='Expert', weightFiles='B2JpsiKs_mu', workingDirectory='./FlavorTagging/TrainedMethods', path=analysis_main):
    """
    Trains or tests all categories at track level except KaonPion, MaximumPstar and FSC which are only at the event level.
    """

    B2INFO('TRACK LEVEL')
    if not Belle2.FileSystem.findFile(workingDirectory):
        B2FATAL('flavorTagger: THE NEEDED DIRECTORY "./FlavorTagging/TrainedMethods" DOES NOT EXIST!')

    ReadyMethods = 0

    # Each category has its own Path in order to be skipped if the corresponding particle list is empty
    trackLevelPathsList = dict()

    for (particleList, category) in trackLevelParticleLists:

        trackLevelPath = category + "TrackLevelPath"

        exec('%s = %s' % (trackLevelPath, 'create_path()'))
        exec('trackLevelPathsList["' + category + '"]=%s' % trackLevelPath)

        SkipEmptyParticleList = register_module("SkimFilter")
        SkipEmptyParticleList.param('particleLists', particleList)
        SkipEmptyParticleList.if_true(trackLevelPathsList[category], AfterConditionPath.CONTINUE)
        path.add_module(SkipEmptyParticleList)

        methodPrefixTrackLevel = weightFiles + 'TrackLevel' + category + 'TMVA'
        targetVariable = 'isRightTrack(' + category + ')'

        if mode == 'Expert':
            if not os.path.isfile(workingDirectory + '/' + methodPrefixTrackLevel + '_1.root'):
                B2FATAL(
                    'Flavor Tagger: ' +
                    particleList +
                    ' Tracklevel was not trained. Weight file ' +
                    methodPrefixTrackLevel +
                    '_1.config not found. Stopped')
            else:
                B2INFO('flavorTagger: Applying MVAExpert ' + methodPrefixTrackLevel + ' .')

                trackLevelPathsList[category].add_module(
                    'MVAExpert',
                    listNames=[particleList],
                    extraInfoName=targetVariable,
                    signalFraction=signalFraction,
                    identifier=workingDirectory +
                    '/' +
                    methodPrefixTrackLevel +
                    "_1.root")

                ReadyMethods += 1

        if mode == 'Sampler':
            if not os.path.isfile(workingDirectory + '/' + methodPrefixTrackLevel + ".root"):

                B2INFO('flavorTagger: file ' + workingDirectory + '/' + methodPrefixTrackLevel + ".root" + ' will be saved.')

                ntuple = register_module('VariablesToNtuple')
                ntuple.param('fileName', workingDirectory + '/' + methodPrefixTrackLevel + ".root")
                ntuple.param('treeName', methodPrefixTrackLevel + "_tree")
                ntuple.param('variables', variables[category] + [targetVariable])
                ntuple.param('particleList', particleList)
                trackLevelPathsList[category].add_module(ntuple)
            else:
                trackLevelPathsList[category].add_module(
                    'MVAExpert',
                    listNames=[particleList],
                    extraInfoName=targetVariable,
                    signalFraction=signalFraction,
                    identifier=workingDirectory +
                    '/' +
                    methodPrefixTrackLevel +
                    "_1.root")
                ReadyMethods += 1

    if ReadyMethods != len(trackLevelParticleLists):
        return False
    else:
        return True


def trackLevelTeacher(weightFiles='B2JpsiKs_mu', workingDirectory='./FlavorTagging/TrainedMethods'):
    """
    Trains or tests all categories at track level except KaonPion, MaximumPstar and FSC which are only at the event level.
    """

    B2INFO('TRACK LEVEL TEACHER')
    if not Belle2.FileSystem.findFile(workingDirectory):
        B2FATAL('flavorTagger: THE NEEDED DIRECTORY "./FlavorTagging/TrainedMethods" DOES NOT EXIST!')

    ReadyMethods = 0

    for (particleList, category) in trackLevelParticleLists:

        methodPrefixTrackLevel = weightFiles + 'TrackLevel' + category + 'TMVA'
        targetVariable = 'isRightTrack(' + category + ')'

        if not os.path.isfile(workingDirectory + '/' + methodPrefixTrackLevel + '_1.root'):

            if os.path.isfile(workingDirectory + '/' + methodPrefixTrackLevel + ".root"):

                B2INFO('flavorTagger: MVA Teacher training' + methodPrefixTrackLevel + ' .')

                trainingOptionsTrackLevel = basf2_mva.GeneralOptions()
                trainingOptionsTrackLevel.m_datafiles = basf2_mva.vector(workingDirectory + '/' + methodPrefixTrackLevel + ".root")
                trainingOptionsTrackLevel.m_treename = methodPrefixTrackLevel + "_tree"
                trainingOptionsTrackLevel.m_weightfile = workingDirectory + '/' + methodPrefixTrackLevel + "_1.root"
                trainingOptionsTrackLevel.m_variables = basf2_mva.vector(*variables[category])
                trainingOptionsTrackLevel.m_target_variable = targetVariable

                basf2_mva.teacher(trainingOptionsTrackLevel, fastBDTCategories)
                basf2_mva.upload(workingDirectory + '/' + methodPrefixTrackLevel + "_1.root", methodPrefixTrackLevel)

            else:

                B2INFO('flavorTagger: trackLevelTeacher did not find ' + workingDirectory + '/' +
                       methodPrefixTrackLevel + ".root" + ' . Please run the flavorTagger in "Sampler" mode.')
        else:

            ReadyMethods += 1

    if ReadyMethods != len(trackLevelParticleLists):
        return False
    else:
        return True


def eventLevel(mode='Expert', weightFiles='B2JpsiKs_mu', workingDirectory='./FlavorTagging/TrainedMethods', path=analysis_main):
    """
    Trains or tests all categories at event level.
    """

    B2INFO('EVENT LEVEL')
    if not Belle2.FileSystem.findFile(workingDirectory):
        B2FATAL('flavorTagger: THE NEEDED DIRECTORY "./FlavorTagging/TrainedMethods" DOES NOT EXIST!')

    ReadyMethods = 0

    # Each category has its own Path in order to be skipped if the corresponding particle list is empty
    eventLevelPathsList = dict()

    for (particleList, category) in eventLevelParticleLists:

        eventLevelPath = category + "EventLevelPath"

        exec('%s = %s' % (eventLevelPath, 'create_path()'))
        exec('eventLevelPathsList["' + category + '"]=%s' % eventLevelPath)

        SkipEmptyParticleList = register_module("SkimFilter")
        SkipEmptyParticleList.param('particleLists', particleList)
        SkipEmptyParticleList.if_true(eventLevelPathsList[category], AfterConditionPath.CONTINUE)
        path.add_module(SkipEmptyParticleList)

        methodPrefixEventLevel = weightFiles + 'EventLevel' + category + 'TMVA'
        targetVariable = 'isRightCategory(' + category + ')'

        if mode == 'Expert':
            if not os.path.isfile(workingDirectory + '/' + methodPrefixEventLevel + '_1.root'):
                B2FATAL(
                    'Flavor Tagger: ' +
                    particleList +
                    ' Tracklevel was not trained. Weight file ' +
                    methodPrefixEventLevel +
                    '_1.config not found. Stopped')
            else:
                B2INFO('flavorTagger: Applying MVAExpert ' + methodPrefixEventLevel + ' .')

                eventLevelPathsList[category].add_module(
                    'MVAExpert',
                    listNames=[particleList],
                    extraInfoName=targetVariable,
                    signalFraction=signalFraction,
                    identifier=workingDirectory +
                    '/' +
                    methodPrefixEventLevel +
                    "_1.root")

                ReadyMethods += 1

        if mode == 'Sampler':
            if not os.path.isfile(workingDirectory + '/' + methodPrefixEventLevel + ".root"):
                B2INFO('flavorTagger: file ' + methodPrefixEventLevel + ".root" + ' will be saved.')

                ntuple = register_module('VariablesToNtuple')
                ntuple.param('fileName', workingDirectory + '/' + methodPrefixEventLevel + ".root")
                ntuple.param('treeName', methodPrefixEventLevel + "_tree")
                ntuple.param('variables', variables[category] + [targetVariable])
                ntuple.param('particleList', particleList)
                eventLevelPathsList[category].add_module(ntuple)
            else:

                eventLevelPathsList[category].add_module(
                    'MVAExpert',
                    listNames=[particleList],
                    extraInfoName=targetVariable,
                    signalFraction=signalFraction,
                    identifier=workingDirectory +
                    '/' +
                    methodPrefixEventLevel +
                    "_1.root")
                ReadyMethods += 1

    if ReadyMethods != len(eventLevelParticleLists):
        return False
    else:
        return True


def eventLevelTeacher(weightFiles='B2JpsiKs_mu', workingDirectory='./FlavorTagging/TrainedMethods', path=analysis_main):
    """
    Trains or tests all categories at track level except KaonPion, MaximumPstar and FSC which are only at the event level.
    """

    B2INFO('EVENT LEVEL TEACHER')
    if not Belle2.FileSystem.findFile(workingDirectory):
        B2FATAL('flavorTagger: THE NEEDED DIRECTORY "./FlavorTagging/TrainedMethods" DOES NOT EXIST!')

    ReadyMethods = 0

    for (particleList, category) in eventLevelParticleLists:

        methodPrefixEventLevel = weightFiles + 'EventLevel' + category + 'TMVA'
        targetVariable = 'isRightCategory(' + category + ')'

        if not os.path.isfile(workingDirectory + '/' + methodPrefixEventLevel + '_1.root'):

            if os.path.isfile(workingDirectory + '/' + methodPrefixEventLevel + ".root"):

                B2INFO('flavorTagger: MVA Teacher training' + methodPrefixEventLevel + ' .')

                trainingOptionsEventLevel = basf2_mva.GeneralOptions()
                trainingOptionsEventLevel.m_datafiles = basf2_mva.vector(workingDirectory + '/' + methodPrefixEventLevel + ".root")
                trainingOptionsEventLevel.m_treename = methodPrefixEventLevel + "_tree"
                trainingOptionsEventLevel.m_weightfile = workingDirectory + '/' + methodPrefixEventLevel + "_1.root"
                trainingOptionsEventLevel.m_variables = basf2_mva.vector(*variables[category])
                trainingOptionsEventLevel.m_target_variable = targetVariable

                basf2_mva.teacher(trainingOptionsEventLevel, fastBDTCategories)
                basf2_mva.upload(workingDirectory + '/' + methodPrefixEventLevel + "_1.root", methodPrefixEventLevel)

            else:

                B2INFO('flavorTagger: eventLevelTeacher did not find ' + methodPrefixEventLevel +
                       ".root" + ' . Please run the flavorTagger in "Sampler" mode.')
        else:

            ReadyMethods += 1

    if ReadyMethods != len(eventLevelParticleLists):
        return False
    else:
        return True


def combinerLevel(mode='Expert', weightFiles='B2JpsiKs_mu', workingDirectory='./FlavorTagging/TrainedMethods',
                  path=analysis_main):
    """
    Trains or tests the combiner according to the selected categories.
    """

    B2INFO('COMBINER LEVEL')

    if not Belle2.FileSystem.findFile(workingDirectory):
        B2FATAL('flavorTagger: THE NEEDED DIRECTORY "./FlavorTagging/TrainedMethods" DOES NOT EXIST!')

    methodPrefixCombinerLevel = weightFiles + 'CombinerLevel' \
        + categoriesCombinationCode

    ReadyTMVAfbdt = False
    ReadyFANNmlp = False

    if mode == 'Sampler':

        B2INFO('flavorTagger: Sampling Data on Combiner Level. File' +
               methodPrefixCombinerLevel + 'TMVA' + ".root" + 'will be saved')

        ntuple = register_module('VariablesToNtuple')
        ntuple.param('fileName', workingDirectory + '/' + methodPrefixCombinerLevel + 'TMVA' + ".root")
        ntuple.param('treeName', methodPrefixCombinerLevel + 'TMVA' + "_tree")
        ntuple.param('variables', variablesCombinerLevel + ['qrCombined'])
        ntuple.param('particleList', "")
        path.add_module(ntuple)

    if TMVAfbdt:

        if mode == 'Expert':
            if not os.path.isfile(workingDirectory + '/' + methodPrefixCombinerLevel + 'TMVA' + '_1.root'):

                B2FATAL('flavorTagger: Combinerlevel was not trained with this combination of categories. Weight file ' +
                        methodPrefixCombinerLevel + 'TMVA' + '_1.config not found. Stopped')
            else:
                B2INFO('flavorTagger: Ready to be used with weightFiles ' +
                       weightFiles + '. The training process has been finished.')
                if mode == 'Expert':
                    B2INFO('flavorTagger: Apply TMVAMethod ' + methodPrefixCombinerLevel + 'TMVA')
                    path.add_module('MVAExpert', listNames=[], extraInfoName='qrCombined' + 'TMVA', signalFraction=signalFraction,
                                    identifier=workingDirectory + '/' + methodPrefixCombinerLevel + 'TMVA' + "_1.root")
                    ReadyTMVAfbdt = True

    if FANNmlp:
        if not isFANNMethodAvailable(workingDirectory + '/' + methodPrefixCombinerLevel + 'FANN'):
            if mode == 'Expert':
                B2FATAL('flavorTagger: Combinerlevel was not trained with this combination of categories. Weight file ' +
                        methodPrefixCombinerLevel + 'FANN' + '_WeightFile.root not found. Stopped')
            else:
                B2INFO('Train FANNMethod on combiner level')
                trainFANNMethod(
                    [],
                    variables=variablesCombinerLevel,
                    target='qrCombined',
                    prefix=methodPrefixCombinerLevel + 'FANN',
                    methods=methodsCombinerFANN,
                    workingDirectory=workingDirectory,
                    path=path,
                )

        else:
            B2INFO('flavorTagger: Ready to be used with weightFiles ' +
                   weightFiles + '. The training process has been finished.')
            if mode == 'Expert' or (mode == 'Teacher' and TMVAfbdt):
                B2INFO('Apply FANNMethod on combiner level')
                applyFANNMethod(
                    [],
                    expertOutputName='qrCombined' + 'FANN',
                    prefix=methodPrefixCombinerLevel + 'FANN',
                    method=methodsCombinerFANN[0][0],
                    workingDirectory=workingDirectory,
                    path=path,
                )
                ReadyFANNmlp = True

            else:
                B2FATAL(
                    'flavorTagger: Combinerlevel was already trained with this combination of categories. Weight file ' +
                    methodPrefixCombinerLevel +
                    'TMVA' +
                    '_1.config or ' +
                    methodPrefixCombinerLevel +
                    '_WeightFile.root has been found. Please use the "Expert" mode')

        if TMVAfbdt and not FANNmlp:
            if ReadyTMVAfbdt:
                return True
        if FANNmlp and not TMVAfbdt:
            if ReadyFANNmlp:
                return True
        if FANNmlp and TMVAfbdt:
            if ReadyFANNmlp and ReadyTMVAfbdt:
                return True


def combinerLevelTeacher(weightFiles='B2JpsiKs_mu', workingDirectory='./FlavorTagging/TrainedMethods',
                         path=analysis_main):
    """
    Trains or tests the combiner according to the selected categories.
    """

    B2INFO('COMBINER LEVEL')

    if not Belle2.FileSystem.findFile(workingDirectory):
        B2FATAL('flavorTagger: THE NEEDED DIRECTORY "./FlavorTagging/TrainedMethods" DOES NOT EXIST!')

    methodPrefixCombinerLevel = weightFiles + 'CombinerLevel' \
        + categoriesCombinationCode

    ReadyTMVAfbdt = False
    ReadyFANNmlp = False

    if not os.path.isfile(workingDirectory + '/' + methodPrefixCombinerLevel + 'TMVA' + '_1.root'):
        if os.path.isfile(workingDirectory + '/' + methodPrefixCombinerLevel + 'TMVA' + '.root'):

            B2INFO('flavorTagger: MVA Teacher training FBDT on Combiner Level')

            trainingOptionsTrackLevel = basf2_mva.GeneralOptions()
            trainingOptionsTrackLevel.m_datafiles = basf2_mva.vector(
                workingDirectory + '/' + methodPrefixCombinerLevel + 'TMVA' + ".root")
            trainingOptionsTrackLevel.m_treename = methodPrefixCombinerLevel + 'TMVA' + "_tree"
            trainingOptionsTrackLevel.m_weightfile = workingDirectory + '/' + methodPrefixCombinerLevel + 'TMVA' + "_1.root"
            trainingOptionsTrackLevel.m_variables = basf2_mva.vector(*variablesCombinerLevel)
            trainingOptionsTrackLevel.m_target_variable = 'qrCombined'
            basf2_mva.teacher(trainingOptionsTrackLevel, fastBDTCombiner)

            basf2_mva.upload(workingDirectory + '/' + methodPrefixCombinerLevel +
                             'TMVA' + "_1.root", methodPrefixCombinerLevel + 'TMVA')

        else:

            B2FATAL('FlavorTagger: combinerLevelTeacher did not find' +
                    methodPrefixCombinerLevel + 'TMVA' + '.root . Please run the flavorTagger in "Sampler" mode.')

    else:
        B2FATAL('flavorTagger: Combinerlevel was already trained with this combination of categories. Weight file ' +
                methodPrefixCombinerLevel + 'TMVA' + '_1.config has been found. Please use the "Expert" mode')


def flavorTagger(
    particleList,
    mode='Expert',
    weightFiles='B2JpsiKs_mu',
    workingDirectory='.',
    combinerMethods=['TMVA-FBDT', 'FANN-MLP'],
    categories=[
        'Electron',
        'IntermediateElectron',
        'Muon',
        'IntermediateMuon',
        'KinLepton',
        'IntermediateKinLepton',
        'Kaon',
        'SlowPion',
        'FastPion',
        'Lambda',
        'FSC',
        'MaximumPstar',
        'KaonPion'],
    belleOrBelle2="Belle2",
    path=analysis_main,
):
    """
      Defines the whole flavor tagging process.
      For each Rest of Event built in the steering file.
      The flavor is predicted by Multivariate Methods trained with Variables and MetaVariables which use
      Tracks, ECL- and KLMClusters from the corresponding RestOfEvent dataobject.
      This function can be used to train or to test the flavorTagger: The available modes are "Teacher" or "Expert".
    """

    if mode != 'Sampler' and mode != 'Teacher' and mode != 'Expert':
        B2FATAL('flavorTagger: Wrong mode given: The available modes are "Sampler", "Teacher" or "Expert"')

    # Directory where the weights of the trained Methods are saved
    # workingDirectory = os.environ['BELLE2_LOCAL_DIR'] + '/analysis/data'

    if not Belle2.FileSystem.findFile(workingDirectory):
        B2FATAL('flavorTagger: THE GIVEN WORKING DIRECTORY "' + workingDirectory + '" DOES NOT EXIST! PLEASE SPECIFY A VALID PATH.')

    if mode == 'Sampler':
        if not Belle2.FileSystem.findFile(workingDirectory + '/FlavorTagging'):
            os.mkdir(workingDirectory + '/FlavorTagging')
            os.mkdir(workingDirectory + '/FlavorTagging/TrainedMethods')
        elif not Belle2.FileSystem.findFile(workingDirectory + '/FlavorTagging/TrainedMethods'):
            os.mkdir(workingDirectory + '/FlavorTagging/TrainedMethods')

    workingDirectory = workingDirectory + '/FlavorTagging/TrainedMethods'

    if len(combinerMethods) < 1 or len(combinerMethods) > 2:
        B2FATAL('flavorTagger: Invalid list of combinerMethods. The available methods are "TMVA-FBDT" and "FANN-MLP"')

    global belleOrBelle2Flag

    if belleOrBelle2 == 'Belle2' or belleOrBelle2 == 'Belle':
        belleOrBelle2Flag = belleOrBelle2
    else:
        B2FATAL('flavorTagger: Wrong argument for belleOrBelle2 given: The available modes are "Belle2" or "Belle"')

    global FANNmlp
    global TMVAfbdt

    FANNmlp = False
    TMVAfbdt = False

    for method in combinerMethods:
        if method == 'TMVA-FBDT':
            TMVAfbdt = True
        elif method == 'FANN-MLP':
            FANNmlp = True
        else:
            B2FATAL('flavorTagger: Invalid list of combinerMethods. The available methods are "TMVA-FBDT" and "FANN-MLP"')

    B2INFO('*** FLAVOR TAGGING ***')
    B2INFO(' ')
    B2INFO('    Working directory is: ' + workingDirectory)
    B2INFO(' ')

    setModeCode(mode)
    setBelleOrBelle2(belleOrBelle2)
    setVariables()

    roe_path = create_path()
    deadEndPath = create_path()

    # Events containing ROE without B-Meson (but not empty) are discarded for training
    if mode == 'Sampler':
        signalSideParticleFilter(particleList, 'hasRestOfEventTracks > 0 and qrCombined > -2', roe_path, deadEndPath)

    # If trigger returns 1 jump into empty path skipping further modules in roe_path
    if mode == 'Expert':
        signalSideParticleFilter(particleList, 'hasRestOfEventTracks > 0', roe_path, deadEndPath)

    # sampler or expert
    if mode == 'Sampler' or mode == 'Expert':
        if WhichCategories(categories):
            if FillParticleLists(mode, roe_path):
                if trackLevel(mode, weightFiles, workingDirectory, roe_path):
                    if eventLevel(mode, weightFiles, workingDirectory, roe_path):
                        combinerLevel(mode, weightFiles, workingDirectory, roe_path)
                        if mode == 'Expert':
                                # Initialation of flavorTaggerInfo dataObject needs to be done in the main path
                            flavorTaggerInfoBuilder = register_module('FlavorTaggerInfoBuilder')
                            path.add_module(flavorTaggerInfoBuilder)
                            roe_path.add_module(flavorTaggerInfoFiller())  # Add FlavorTag Info filler to roe_path

        # Removes EventExtraInfos and ParticleExtraInfos of the EventParticleLists
        roe_path.add_module(RemoveExtraInfoModule())
        path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

    if mode == 'Teacher':
        if trackLevelTeacher(weightFiles, workingDirectory):
            if eventLevelTeacher(weightFiles, workingDirectory, path):
                combinerLevelTeacher(weightFiles, workingDirectory, path)
