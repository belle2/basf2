#!/usr/bin/env python
# -*- coding: utf-8 -*-

# *********   Flavor  Tagging   *********
# * Authors: Thomas Keck & Moritz Gelb  *
# * The whole flavor tagging can be     *
# * done in this script.                *
# ***************************************


from basf2 import *
from modularAnalysis import *
from ROOT import Belle2

main = create_path()
main.add_module(register_module('RootInput'))
loadReconstructedParticles(path=main)

# Signal side B_sig
selectParticle('mu+', 'muid >= 0.5', path=main)
selectParticle('pi+', 'piid >= 0.5', path=main)

makeParticle('J/psi -> mu+ mu-', '3.0<=M<=3.2', path=main)
makeParticle('K_S0 -> pi+ pi-', '0.25<=M<=0.75', path=main)
makeParticle('B0 -> K_S0 J/psi', '5.2 <= M <= 5.4', path=main)
matchMCTruth('B0', path=main)

# Tag side B_tag
buildRestOfEvent('B0', path=main)
buildContinuumSuppression('B0', path=main)
roe_path = create_path()

# Variables for categories on track level - are defined in variables.cc
variables = dict()
variables['Electron'] = ['p_CMS', 'pt_CMS', 'cosTheta', 'p', 'pt', 'chiProb', 'charge', 'mRecoil', 'p_miss']
variables['Muon'] = ['p_CMS', 'pt_CMS', 'cosTheta', 'p', 'pt', 'chiProb', 'charge', 'mRecoil', 'p_miss']
variables['Kaon'] = ['p_CMS', 'pt_CMS', 'cosTheta', 'p', 'pt', 'chiProb', 'Kid', 'Kid_dEdx', 'Kid_TOP', 'Kid_ARICH', 'charge', 'isThereAKShortinRoe']
variables['SlowPion'] = ['p_CMS', 'pt_CMS', 'cosTheta', 'p', 'pt', 'chiProb', 'pi_vs_edEdxid', 'charge', 'cosTPTO']
#Lambdas are missing

#Please choose method:
methods = [('FastBDT', 'Plugin', '!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3')]
#methods = [("Fisher", "Fisher", "H:!V:Fisher:VarTransform=None:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=50:NsmoothMVAPdf=10")]
#methods = [("BDTGradient", "BDT", "!H:!V:CreateMVAPdfs:NTrees=100:BoostType=Grad:Shrinkage=0.10:UseBaggedGrad:GradBaggingFraction=0.5:nCuts=200:MaxDepth=2")]
#methods = [("PDEFoamBoost", "PDEFoam", "!H:!V:CreateMVAPdfs:Boost_Num=10:Boost_Transform=linear:SigBgSeparate=F:MaxDepth=4:UseYesNoCell=T:DTLogic=MisClassificationError:FillFoamWithOrigWeights=F:TailCut=0:nActiveCells=500:nBin=20:Nmin=400:Kernel=None:Compress=T")]

trackLevelReady = True
trackLevelParticles = [('e+', 'Electron'), ('mu+', 'Muon'), ('K+', 'Kaon'), ('pi+', 'SlowPion')]

for symbol, category in trackLevelParticles:
    particleList = symbol + ':ROE'
    methodPrefix = category
    targetVariable = 'is' + category + 'FromB'

    particleListKShort = 'pi+' + ':inKaonRoe'

    # Select Particles in ROE for different categories of flavour tagging.
    selectParticle(particleList, 'isInRestOfEvent > 0.5', path=roe_path)
    if symbol == 'K+':
        applyCuts('K+:ROE', '0.1<Kid', path=roe_path)
        selectParticle(particleListKShort, 'isInRestOfEvent > 0.5', path=roe_path)
        makeParticle('K_S0:ROE -> pi+:inKaonRoe pi-:inKaonRoe', '0.40<=M<=0.60', path=roe_path)
        fitVertex('K_S0:ROE', 0.01)
        #modify confidence level?!

    if not isTMVAMethodAvailable(methodPrefix):
        trainTMVAMethod(particleList, variables=variables[category], target=targetVariable, methods=methods, prefix=methodPrefix, path=roe_path)
        trackLevelReady = False
    else:
        applyTMVAMethod(particleList, prefix=methodPrefix, signalProbabilityName=targetVariable, method=methods[0][0], path=roe_path)

eventLevelReady = trackLevelReady
if trackLevelReady:
    variables = ['bestQRKaon', 'bestQRSlowPion', 'bestQRElectron', 'bestQRMuon']
    if not isTMVAMethodAvailable('B0Tagger'):
        trainTMVAMethod([], variables=variables, target='isMajorityInRestOfEventFromB0', prefix='B0Tagger', methods=methods, path=roe_path)
        eventLevelReady = False
    else:
        applyTMVAMethod([], signalProbabilityName='isMajorityInRestOfEventFromB0', prefix='B0Tagger', method=methods[0][0], path=roe_path)

    if not isTMVAMethodAvailable('B0barTagger'):
        trainTMVAMethod([], variables=variables, target='isMajorityInRestOfEventFromB0bar', prefix='B0barTagger', methods=methods, path=roe_path)
        eventLevelReady = False
    else:
        applyTMVAMethod([], signalProbabilityName='isMajorityInRestOfEventFromB0bar', prefix='B0barTagger', method=methods[0][0], path=roe_path)

    class RemoveExtraInfoModule(Module):
        def event(self):
            for symbol, _ in trackLevelParticles:
                plist = Belle2.PyStoreObj(symbol + ':ROE')
                for i in range(0, plist.obj().getListSize()):
                    particle = plist.obj().getParticle(i)
                    particle.removeExtraInfo()

    roe_path.add_module(RemoveExtraInfoModule())


if eventLevelReady:
    class MoveTaggerInformationToBExtraInfoModule(Module):
        def event(self):
            roe = Belle2.PyStoreObj("RestOfEvent")
            info = Belle2.PyStoreObj("EventExtraInfo")
            b_probability = info.obj().getExtraInfo("isMajorityInRestOfEventFromB0")
            bbar_probability = info.obj().getExtraInfo("isMajorityInRestOfEventFromB0bar")
            particle = roe.obj().getRelated("Particles")
            particle.addExtraInfo("b_probability", b_probability)
            particle.addExtraInfo("bbar_probability", bbar_probability)
            info.obj().removeExtraInfo()

    roe_path.add_module(MoveTaggerInformationToBExtraInfoModule())

main.for_each('RestOfEvent', 'RestOfEvents', roe_path)

if eventLevelReady:
    variablesToNTuple('B0', ['getExtraInfo(b_probability)', 'getExtraInfo(bbar_probability)'], 'TaggingInformation', 'B0_B0bar_final.root', path=main)

main.add_module(register_module('ProgressBar'))
process(main)
print statistics

B2INFO("")
B2INFO("Training completed. Run 'show_tmva_results TMVA_1_vs_0.root' to view detailed"
       " information about the trained methods.")
