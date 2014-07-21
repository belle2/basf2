#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Thomas Keck & Moritz Gelb

from basf2 import *
from modularAnalysis import *
from ROOT import Belle2

main = create_path()
main.add_module(register_module('RootInput'))
loadReconstructedParticles(path=main)

# Reconstruct signal side
selectParticle('mu+', 'muid >= 0.5', path=main)
selectParticle('pi+', 'piid >= 0.5', path=main)

makeParticle('J/psi -> mu+ mu-', '3.0<=M<=3.2', path=main)
makeParticle('K_S0 -> pi+ pi-', '0.25<=M<=0.75', path=main)
makeParticle('B0 -> K_S0 J/psi', '5.2 <= M <= 5.4', path=main)
matchMCTruth('B0', path=main)

# Apply flavour tagging to RestOfEvent
buildRestOfEvent('B0', path=main)
roe_path = create_path()

trackLevelReady = True
trackLevelParticles = [('e+', 'Electron'), ('mu+', 'Muon'), ('K+', 'Kaon'), ('pi+', 'SlowPion')]
for symbol, category in trackLevelParticles:
    particleList = symbol + ':ROE'
    methodPrefix = category
    targetVariable = 'is' + category + 'FromB'

    # Select Particles in ROE for different categories of flavour tagging.
    selectParticle(particleList, 'isInRestOfEvent > 0.5', path=roe_path)
    if not isTMVAMethodAvailable(methodPrefix):
        variables = ['p_CMS', 'pt_CMS', 'cosTheta', 'p', 'pt', 'chiProb', 'Kid', 'Kid_dEdx', 'Kid_TOP', 'Kid_ARICH', 'piid', 'piid_dEdx', 'piid_TOP', 'piid_ARICH', 'charge']
        trainTMVAMethod(particleList, variables=variables, target=targetVariable, prefix=methodPrefix, path=roe_path)
        trackLevelReady = False
    else:
        applyTMVAMethod(particleList, prefix=methodPrefix, signalProbabilityName=targetVariable, path=roe_path)

eventLevelReady = trackLevelReady
if trackLevelReady:
    variables = ['bestQRKaon', 'bestQRSlowPion', 'bestQRElectron', 'bestQRMuon']
    if not isTMVAMethodAvailable('B0Tagger'):
        trainTMVAMethod([], variables=variables, target='isMajorityInRestOfEventFromB0', prefix='B0Tagger', path=roe_path)
        eventLevelReady = False
    else:
        applyTMVAMethod([], signalProbabilityName='isMajorityInRestOfEventFromB0', prefix='B0Tagger', path=roe_path)

    if not isTMVAMethodAvailable('B0barTagger'):
        trainTMVAMethod([], variables=variables, target='isMajorityInRestOfEventFromB0', prefix='B0barTagger', path=roe_path)
        eventLevelReady = False
    else:
        applyTMVAMethod([], signalProbabilityName='isMajorityInRestOfEventFromB0', prefix='B0barTagger', path=roe_path)

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
            bbar_probability = info.obj().getExtraInfo("isMajorityInRestOfEventFromB0")
            particle = roe.obj().getRelated("Particles")
            particle.addExtraInfo("b_probability", b_probability)
            particle.addExtraInfo("bbar_probability", bbar_probability)

    roe_path.add_module(MoveTaggerInformationToBExtraInfoModule())

main.for_each('RestOfEvent', 'RestOfEvents', roe_path)

if eventLevelReady:
    variablesToNTuple('B0', ['getExtraInfo(b_probability)', 'getExtraInfo(bbar_probability)'], path=main)

main.add_module(register_module('ProgressBar'))
process(main)
print statistics

B2INFO("")
B2INFO("Training completed. Run 'show_tmva_results TMVA_1_vs_0.root' to view detailed"
       " information about the trained methods.")
