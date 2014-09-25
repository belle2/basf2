#!/usr/bin/env python
# -*- coding: utf-8 -*-

# *************  Flavor Tagging   ************
# * Authors: Fernando Abudinen, Moritz Gelb  *
# *.....     and Thomas Keck                 *
# * The whole flavor tagging can be done in  *
# * this script.                             *
# ********************************************

# Specific Variables and MetaVariables are used.

from basf2 import *
from modularAnalysis import *
import variables as mc_variables
from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True
PyConfig.StartGuiThread = False
import ROOT
from ROOT import Belle2
import os
from array import array

main = create_path()
main.add_module(register_module('RootInput'))
main.add_module(register_module('Gearbox'))
loadReconstructedParticles(path=main)

# Signal side B_sig
selectParticle('mu+', 'muid >= 0.1', path=main)
selectParticle('pi+', 'piid >= 0.1', path=main)

reconstructDecay('J/psi -> mu+ mu-', '3.0<=M<=3.2', 1, path=main)
reconstructDecay('K_S0 -> pi+ pi-', '0.25<=M<=0.75', 1, path=main)
reconstructDecay('B0 -> K_S0 J/psi', '5.2 <= M <= 5.4', 1, path=main)

# Use only correctly reconstructed B_sig
matchMCTruth('B0', path=main)
applyCuts('B0', 'isSignal > 0.5', path=main)

# Tag side B_tag
buildRestOfEvent('B0', path=main)
buildContinuumSuppression('B0', path=main)
roe_path = create_path()


# Variables for categories on track level
variables_TL = dict()
variables_TL['Electron'] = [
    'p_CMS',
    'pt_CMS',
    'p',
    'pt',
    'missing_P_CMS',
    'CosTheta_CMS_missing',
    'RecoilMassTagSide',
    'EW90',
    'eid',
    'eid_dEdx',
    'eid_TOP',
    'eid_ARICH',
    'eid_ECL',
]
variables_TL['IntermediateElectron'] = variables_TL['Electron']
variables_TL['Muon'] = [
    'p_CMS',
    'pt_CMS',
    'p',
    'pt',
    'missing_P_CMS',
    'CosTheta_CMS_missing',
    'RecoilMassTagSide',
    'EW90',
    'muid',
    'muid_dEdx',
    'muid_TOP',
    'muid_ARICH',
]
variables_TL['IntermediateMuon'] = variables_TL['Muon']
variables_TL['Kaon'] = [
    'p_CMS',
    'pt_CMS',
    'cosTheta',
    'pt',
    'Kid',
    'Kid_dEdx',
    'Kid_TOP',
    'Kid_ARICH',
    'charge',
    'NumberOfKShortinRemainingROEKaon',
    'ptTracksRoe',
    'distance',
]
variables_TL['SlowPion'] = [
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
    'charge',
    'Kid',
    'eid',
]
variables_TL['FastPion'] = variables_TL['SlowPion']
variables_TL['Lambda'] = [
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


methods = [('FastBDT', 'Plugin', '!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3')]
# methods = [("Fisher", "Fisher", "H:!V:Fisher:VarTransform=None:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=50:NsmoothMVAPdf=10")]
# methods = [("BDTGradient", "BDT", "!H:!V:CreateMVAPdfs:NTrees=100:BoostType=Grad:Shrinkage=0.10:UseBaggedGrad:GradBaggingFraction=0.5:nCuts=200:MaxDepth=2")]
# methods = [("PDEFoamBoost", "PDEFoam", "!H:!V:CreateMVAPdfs:Boost_Num=10:Boost_Transform=linear:SigBgSeparate=F:MaxDepth=4:UseYesNoCell=T:DTLogic=MisClassificationError:FillFoamWithOrigWeights=F:TailCut=0:nActiveCells=500:nBin=20:Nmin=400:Kernel=None:Compress=T")]

# Directory to save the weights and the .config files
workingDirectory = Belle2.FileSystem.findFile('/analysis/data/FlavorTagging/TrainedMethods')

print 'FLAVOR TAGGING'
print 'Working directory is: ' + workingDirectory

trackLevelReady = True
trackLevelParticles = [('e+', 'Electron'), ('e+', 'IntermediateElectron'), ('mu+', 'Muon'), ('mu+', 'IntermediateMuon'),
                       ('K+', 'Kaon'), ('pi+', 'SlowPion'), ('pi+', 'FastPion'), ('Lambda0', 'Lambda')]
# SignalFraction to calculate probability, -2 if training signal/background ratio should be used
signalFraction = -2

print 'TRACK LEVEL'

for (symbol, category) in trackLevelParticles:
    particleList = symbol + ':ROE'
    methodPrefix_TL = 'TMVA_' + category + '_TL'
    targetVariable = 'IsFromB(' + category + ')'

    # Select particles in ROE for different categories of flavour tagging
    if symbol != 'Lambda0':
        selectParticle(particleList, 'isInRestOfEvent > 0.5', path=roe_path)

    # Check for K short in event
    if symbol == 'K+':
        applyCuts('K+:ROE', '0.1<Kid', path=roe_path)  # Precut done to prevent from overtraining, might be redundant
        selectParticle('pi+:inKaonRoe', 'isInRestOfEvent > 0.5', path=roe_path)
        reconstructDecay('K_S0:ROEKaon -> pi+:inKaonRoe pi-:inKaonRoe', '0.40<=M<=0.60', 1, path=roe_path)
        fitVertex('K_S0:ROEKaon', 0.01, fitter='kfitter', path=roe_path)
        # modify confidence level?!

        # Check for Lambda0 & K short in event
    if symbol == 'Lambda0':
        selectParticle('pi+:inLambdaRoe', 'isInRestOfEvent > 0.5', path=roe_path)
        selectParticle('p+:inLambdaRoe', 'isInRestOfEvent > 0.5', path=roe_path)
        reconstructDecay('Lambda0:ROE -> pi-:inLambdaRoe p+:inLambdaRoe', '', 1, path=roe_path)
        reconstructDecay('K_S0:ROELambda -> pi+:inLambdaRoe pi-:inLambdaRoe', '0.40<=M<=0.60', 1, path=roe_path)

        fitVertex('Lambda0:ROE', 0.01, fitter='kfitter', path=roe_path)
        matchMCTruth('Lambda0:ROE', path=roe_path)
        fitVertex('K_S0:ROELambda', 0.01, fitter='kfitter', path=roe_path)

    if not isTMVAMethodAvailable(workingDirectory + '/' + methodPrefix_TL):
        print 'PROCESSING: trainTMVAMethod on track level: ' + methodPrefix_TL
        trainTMVAMethod(
            particleList,
            variables=variables_TL[category],
            target=targetVariable,
            methods=methods,
            prefix=methodPrefix_TL,
            workingDirectory=workingDirectory,
            path=roe_path,
        )
        trackLevelReady = False
    else:
        print 'PROCESSING: applyTMVAMethod on track level: ' + methodPrefix_TL
        applyTMVAMethod(
            particleList,
            prefix=methodPrefix_TL,
            signalProbabilityName=targetVariable,
            method=methods[0][0],
            signalFraction=signalFraction,
            workingDirectory=workingDirectory,
            path=roe_path,
        )

eventLevelReady = trackLevelReady


class RemoveExtraInfoModule(Module):
    # Removes the variables added as Extrainfo to the track level particles
    def event(self):
        for (symbol, _) in trackLevelParticles:
            plist = Belle2.PyStoreObj(symbol + ':ROE')
            for i in range(0, plist.obj().getListSize()):
                particle = plist.obj().getParticle(i)
                particle.removeExtraInfo()


combinerLevelReady = eventLevelReady

if combinerLevelReady:
    print 'COMBINER LEVEL'
    variables = \
        ['InputQrOf(e+:ROE, IsRightClass(Electron, e+:ROE, IsFromB(Electron)), IsFromB(Electron))',
         'InputQrOf(e+:ROE, IsRightClass(Electron, e+:ROE, IsFromB(Electron)), IsFromB(IntermediateElectron))',
         'InputQrOf(mu+:ROE, IsRightClass(Muon, mu+:ROE, IsFromB(Muon)), IsFromB(Muon))',
         'InputQrOf(mu+:ROE, IsRightClass(Muon, mu+:ROE, IsFromB(Muon)), IsFromB(IntermediateMuon))',
         'InputQrOf(K+:ROE, IsRightClass(Kaon, K+:ROE, IsFromB(Kaon)), IsFromB(Kaon))',
         'InputQrOf(pi+:ROE, IsRightClass(SlowPion, pi+:ROE, IsFromB(SlowPion)), IsFromB(SlowPion))',
         'InputQrOf(pi+:ROE, IsRightClass(SlowPion, pi+:ROE, IsFromB(SlowPion)), IsFromB(FastPion))',
         'InputQrOf(Lambda0:ROE, IsRightClass(Lambda, Lambda0:ROE, IsFromB(Lambda)), IsFromB(Lambda))'
         ]

    method_Combiner = [('FastBDT', 'Plugin', '!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3')]

            # B0Tagger
    if not isTMVAMethodAvailable(workingDirectory + '/' + 'B0Tagger'):
        print 'PROCESSING: trainTMVAMethod on combiner level: B0'
        trainTMVAMethod(
            [],
            variables=variables,
            # change target
            # target='qr_Combined',
            target='targetB0Tagger',
            prefix='B0Tagger',
            methods=method_Combiner,
            workingDirectory=workingDirectory,
            path=roe_path,
        )
        combinerLevelReady = False
    else:
        print 'PROCESSING: applyTMVAMethod on combiner level: B0'
        applyTMVAMethod(
            [],
            signalProbabilityName='targetB0Tagger',
            prefix='B0Tagger',
            signalClass=1,
            method=method_Combiner[0][0],
            signalFraction=signalFraction,
            workingDirectory=workingDirectory,
            path=roe_path,
        )

            # B0BarTagger
    if not isTMVAMethodAvailable(workingDirectory + '/' + 'B0BarTagger'):
        print 'PROCESSING: trainTMVAMethod on combiner level: B0Bar'
        trainTMVAMethod(
            [],
            variables=variables,
            # change target
            #target='qr_Combined',
            target='targetB0BarTagger',
            prefix='B0BarTagger',
            methods=method_Combiner,
            workingDirectory=workingDirectory,
            path=roe_path,
        )
        combinerLevelReady = False
    else:
        print 'PROCESSING: applyTMVAMethod on combiner level: B0Bar'
        applyTMVAMethod(
            [],
            signalProbabilityName='targetB0BarTagger',
            prefix='B0BarTagger',
            signalClass=1,
            method=method_Combiner[0][0],
            signalFraction=signalFraction,
            workingDirectory=workingDirectory,
            path=roe_path,
        )

if combinerLevelReady:
    class MoveTaggerInformationToBExtraInfoModule(Module):
        # Adds the flavor tagging information (q*r) from the MC and from the Combiner as ExtraInfo to the reconstructed B0 particle

        def event(self):
            roe = Belle2.PyStoreObj('RestOfEvent')
            info = Belle2.PyStoreObj('EventExtraInfo')
            B0_prob = info.obj().getExtraInfo('targetB0Tagger')
            B0bar_prob = info.obj().getExtraInfo('targetB0BarTagger')
            particle = roe.obj().getRelated('Particles')
            particle.addExtraInfo('B0_prob', B0_prob)
            particle.addExtraInfo('B0bar_prob', B0bar_prob)
            #instance = Belle2.PyStoreObj('EventExtraInfo')
            #someParticle = Belle2.Particle(None)
            #qr_Combined = 2 * (info.obj().getExtraInfo('qr_Combined') - 0.5)
            #qr_MC = 2 * (mc_variables.variables.evaluate('qr_Combined',
            #             someParticle) - 0.5)
            #particle = roe.obj().getRelated('Particles')
            #particle.addExtraInfo('qr_Combined', qr_Combined)
            #particle.addExtraInfo('qr_MC', qr_MC)
            info.obj().removeExtraInfo()

    roe_path.add_module(MoveTaggerInformationToBExtraInfoModule())  # move and remove extraInfo
    roe_path.add_module(RemoveExtraInfoModule())
else:
    roe_path.add_module(RemoveExtraInfoModule())  # only remove extra info

main.for_each('RestOfEvent', 'RestOfEvents', roe_path)

if combinerLevelReady and not Belle2.FileSystem.findFile(workingDirectory + '/B0_B0bar_final.root'):
    variablesToNTuple('B0', ['getExtraInfo(B0_prob)', 'getExtraInfo(B0bar_prob)', 'isSignal', 'mcPDG'], 'TaggingInformation', workingDirectory + '/B0_B0bar_final.root', path=main)


main.add_module(register_module('ProgressBar'))
process(main)
print statistics

B2INFO('')
B2INFO("Training completed. Run 'show_tmva_results TMVA_1_vs_0.root' to view detailed information about the trained methods.")
