#!/usr/bin/env python
# -*- coding: utf-8 -*-

# *********   Flavor  Tagging   *********    *
# * Authors: Fernando Abudinen, Moritz Gelb  *
# *.....     and Thomas Keck                 *
# * The whole flavor tagging can be          *
# * done in this script.                     *
# ***************************************

from basf2 import *
from modularAnalysis import *
from ROOT import Belle2
import os

main = create_path()
main.add_module(register_module('RootInput'))
loadReconstructedParticles(path=main)

# Signal side B_sig
selectParticle('mu+', 'muid >= 0.5', path=main)
selectParticle('pi+', 'piid >= 0.5', path=main)

reconstructDecay('J/psi -> mu+ mu-', '3.0<=M<=3.2', 1, path=main)
reconstructDecay('K_S0 -> pi+ pi-', '0.25<=M<=0.75', 1, path=main)
reconstructDecay('B0 -> K_S0 J/psi', '5.2 <= M <= 5.4', 1, path=main)
matchMCTruth('B0', path=main)

# Tag side B_tag
buildRestOfEvent('B0', path=main)
buildContinuumSuppression('B0', path=main)
roe_path = create_path()

# Variables for categories on track level - are defined in variables.cc
variables_TL = dict()
variables_TL['Electron'] = ['p_CMS', 'pt_CMS', 'p', 'pt', 'eid']
variables_TL['Muon'] = ['p_CMS', 'pt_CMS', 'p', 'pt', 'muid']
variables_TL['Kaon'] = [
    'p_CMS',
    'pt_CMS',
    'cosTheta',
    'pt',
    'Kid',
    'Kid_dEdx',
    'Kid_TOP',
    'isThereAKShortinRoe',
    ]
variables_TL['SlowPion'] = [
    'p_CMS',
    'pt_CMS',
    'cosTheta',
    'p',
    'pt',
    'pi_vs_edEdxid',
    'cosTPTO',
    ]
# Lambdas are missing

# Please choose method:
methods = [('FastBDT', 'Plugin',
           '!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3'
           )]
# methods = [("Fisher", "Fisher", "H:!V:Fisher:VarTransform=None:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=50:NsmoothMVAPdf=10")]
# methods = [("BDTGradient", "BDT", "!H:!V:CreateMVAPdfs:NTrees=100:BoostType=Grad:Shrinkage=0.10:UseBaggedGrad:GradBaggingFraction=0.5:nCuts=200:MaxDepth=2")]
# methods = [("PDEFoamBoost", "PDEFoam", "!H:!V:CreateMVAPdfs:Boost_Num=10:Boost_Transform=linear:SigBgSeparate=F:MaxDepth=4:UseYesNoCell=T:DTLogic=MisClassificationError:FillFoamWithOrigWeights=F:TailCut=0:nActiveCells=500:nBin=20:Nmin=400:Kernel=None:Compress=T")]

# Directory to save the weights and the .config files
workingDirectory = os.environ['BELLE2_LOCAL_DIR'] \
    + '/analysis/modules/FlavorTagging/TrainedMethods'

trackLevelReady = True
trackLevelParticles = [('e+', 'Electron'), ('mu+', 'Muon'), ('K+', 'Kaon'),
                       ('pi+', 'SlowPion')]
# signalFraction to calculate probability, -2 if training signal/background ratio should be used
signalFraction = -2

for (symbol, category) in trackLevelParticles:
    particleList = symbol + ':ROE'
    methodPrefix_TL = 'TMVA_' + category + '_TL'
    targetVariable = 'is' + category + 'FromB'

    particleListKShort = 'pi+' + ':inKaonRoe'

    # Select Particles in ROE for different categories of flavour tagging.
    selectParticle(particleList, 'isInRestOfEvent > 0.5', path=roe_path)
    if symbol == 'K+':
        applyCuts('K+:ROE', '0.1<Kid', path=roe_path)
        selectParticle(particleListKShort, 'isInRestOfEvent > 0.5',
                       path=roe_path)
        reconstructDecay('K_S0:ROE -> pi+:inKaonRoe pi-:inKaonRoe',
                         '0.40<=M<=0.60', 1, path=roe_path)
        fitVertex('K_S0:ROE', 0.01)
        # modify confidence level?!

    if not isTMVAMethodAvailable(workingDirectory + '/' + methodPrefix_TL):
        print isTMVAMethodAvailable(workingDirectory + '/' + methodPrefix_TL)
        print workingDirectory + '/' + methodPrefix_TL
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
eventLevelParticles = [('e+', 'Electron'), ('mu+', 'Muon'), ('K+', 'Kaon'),
                       ('pi+', 'SlowPion')]

variables_EL = dict()
variables_EL['Electron'] = ['p_CMS_Electron', 'mRecoilBtagElectron',
                            'p_CMS_missingElectron', 'cosTheta_missingElectron'
                            , 'EW90Electron']
variables_EL['Muon'] = ['p_CMS_Muon', 'mRecoilBtagMuon', 'p_CMS_missingMuon',
                        'cosTheta_missingMuon', 'EW90Muon']
variables_EL['Kaon'] = ['bestQRKaon', 'p_CMS_Kaon']  # TODO More Event Level Variables
variables_EL['SlowPion'] = ['bestQRSlowPion', 'p_CMS_SlowPion']  #   TODO More Event Level Variables
# Lambdas are missing

if eventLevelReady:
    for (symbol, category) in eventLevelParticles:
        methodPrefix_EL = 'TMVA_' + category + '_EL'
        targetVariable = 'is' + category + 'RightClass'
        if not isTMVAMethodAvailable(workingDirectory + '/' + methodPrefix_EL):
            print 'hi'
            trainTMVAMethod(
                [],
                variables=variables_EL[category],
                target=targetVariable,
                prefix=methodPrefix_EL,
                methods=methods,
                workingDirectory=workingDirectory,
                path=roe_path,
                )
            eventLevelReady = False
        else:
            applyTMVAMethod(
                [],
                prefix=methodPrefix_EL,
                signalProbabilityName=targetVariable,
                method=methods[0][0],
                signalFraction=signalFraction,
                workingDirectory=workingDirectory,
                path=roe_path,
                )

combinerLevelReady = eventLevelReady
if combinerLevelReady:
    variables = ['bestQRKaon', 'bestQRSlowPion', 'bestQRElectron', 'bestQRMuon'
                 ]
    if not isTMVAMethodAvailable(workingDirectory + '/' + 'B0Tagger'):
        trainTMVAMethod(
            [],
            variables=variables,
            target='qr_Combined',
            prefix='B0Tagger',
            methods=methods,
            workingDirectory=workingDirectory,
            path=roe_path,
            )
        combinerLevelReady = False
    else:
        applyTMVAMethod(
            [],
            signalProbabilityName='qr_Combined',
            prefix='B0Tagger',
            method=methods[0][0],
            signalFraction=signalFraction,
            workingDirectory=workingDirectory,
            path=roe_path,
            )


    class RemoveExtraInfoModule(Module):

        def event(self):
            for (symbol, _) in trackLevelParticles:
                plist = Belle2.PyStoreObj(symbol + ':ROE')
                for i in range(0, plist.obj().getListSize()):
                    particle = plist.obj().getParticle(i)
                    particle.removeExtraInfo()


    roe_path.add_module(RemoveExtraInfoModule())

if combinerLevelReady:


    class MoveTaggerInformationToBExtraInfoModule(Module):

        def event(self):
            roe = Belle2.PyStoreObj('RestOfEvent')
            info = Belle2.PyStoreObj('EventExtraInfo')
            qr_Combined = info.obj().getExtraInfo('qr_Combined')
            particle = roe.obj().getRelated('Particles')
            particle.addExtraInfo('qr_Combined', qr_Combined)
            info.obj().removeExtraInfo()


    roe_path.add_module(MoveTaggerInformationToBExtraInfoModule())

main.for_each('RestOfEvent', 'RestOfEvents', roe_path)

if combinerLevelReady:
    variablesToNTuple('B0', 'getExtraInfo(qr_Combined)', 'TaggingInformation',
                      'B0_B0bar_final.root', path=main)

main.add_module(register_module('ProgressBar'))
process(main)
print statistics

B2INFO('')
B2INFO("Training completed. Run 'show_tmva_results TMVA_1_vs_0.root' to view detailed information about the trained methods."
       )
