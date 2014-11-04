#!/usr/bin/env python
# -*- coding: utf-8 -*-

# *************  Flavor Tagging   ************
# * Authors: Fernando Abudinen, Moritz Gelb  *
# *.....     and Thomas Keck                 *
# * The whole flavor tagging can be done in  *
# * this script.                             *
# ********************************************

from basf2 import *
from modularAnalysis import *
import variables as mc_variables
from ROOT import Belle2
from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True
PyConfig.StartGuiThread = False
import ROOT
from array import array
import os

# Directory to save the weights and the .config files
workingDirectory = Belle2.FileSystem.findFile('/analysis/data/FlavorTagging/RUN16')


class RemoveEmptyROEModule(Module):

    """
    Returns 1 if current ROE is empty, 0 otherwise.
    For the later plotting we have to set a dummy combiner output on the B0_sig.
    Combiner-network output is set to 0.5, respectively 1 for qr-value (meaning no flavour information).
    """
    def event(self):
        self.return_value(0)
        roe = Belle2.PyStoreObj('RestOfEvent')
        B0 = roe.obj().getRelated('Particles')
        if mc_variables.variables.evaluate('isRestOfEventEmpty', B0) == -2:
            B2INFO('FOUND NO B0 IN ROE! NOT USED FOR TRAINING! COMBINER OUTPUT IS MANUALLY SET')
            B0.addExtraInfo('B0_prob', 0.5)
            B0.addExtraInfo('B0bar_prob', 0.5)
            B0.addExtraInfo('qr_Combined', 0.0)
            B0.addExtraInfo('qr_MC', -999)
            self.return_value(1)

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

# Make a cut for RoE without tracks/clusters on tagside
# Problem is that roe_path does not anymore work on this B0 list. Instead with a lot of Roe's ---
#cutAndCopyList('B0', 'B0:UnCut',  '0.0<=CutOnB0<=1.0', path=main)
#print 'Now on cutted List for B0'
#buildRestOfEvent('B0', path=main)
#applyCuts('B0:UnCut', 'CutOnB0 > -0.5', path=main)


roe_path = create_path()
emptypath = create_path()

#make instance
ROEEmptyTrigger = RemoveEmptyROEModule()

# If trigger returns 1 jump into empty path skipping further modules in roe_path
roe_path.add_module(ROEEmptyTrigger)
ROEEmptyTrigger.if_true(emptypath)


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
    'eid_ECL', ]
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
    'muid_ARICH', ]
variables['IntermediateMuon'] = variables['Muon']
variables['Kaon'] = [
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
    'distance', ]
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
    'charge',
    'Kid',
    'eid', ]
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
    'distance', ]
# distance variable: we assume B0 and daughter of B0 lifetimes (Lambda_c ) are much shorter than Lambda0 lifetime.

# FastBDT Method:
# SignalProbability to -2
# Empirically set NbinsMVAPdf to 50 if less than 100k signal events, 100 if more.
# If doing this, be aware of binning effects. Especially if doing later plots with less bins than here!
# E.g. now 100 bins in methods, but 36 in BellePlot. It may happen that one bigger bin corresponds to
# 3 and sometimes to only 2 of the former bins, and has more/less entries. But this does not correspond
# to reality and is simply an effect.

# For Neurobayes Method see below!

# Please choose method:
#methods = [('FastBDT', 'Plugin', 'CreateMVAPdfs:NbinsMVAPdf=100:!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3')]
# methods = [('FastBDT', 'Plugin', '!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3')]
# methods = [("Fisher", "Fisher", "H:!V:Fisher:VarTransform=None:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=50:NsmoothMVAPdf=10")]
# methods = [("BDTGradient", "BDT", "!H:!V:CreateMVAPdfs:NTrees=100:BoostType=Grad:Shrinkage=0.10:UseBaggedGrad:GradBaggingFraction=0.5:nCuts=200:MaxDepth=2")]
# methods = [("PDEFoamBoost", "PDEFoam", "!H:!V:CreateMVAPdfs:Boost_Num=10:Boost_Transform=linear:SigBgSeparate=F:MaxDepth=4:UseYesNoCell=T:DTLogic=MisClassificationError:FillFoamWithOrigWeights=F:TailCut=0:nActiveCells=500:nBin=20:Nmin=400:Kernel=None:Compress=T")]


B2INFO('*** FLAVOR TAGGING ***')
B2INFO(' ')
B2INFO('    Working directory is: ' + workingDirectory)
B2INFO(' ')
B2INFO('TRACK LEVEL')

trackLevelReady = True
trackLevelParticles = [
    ('e+', 'Electron'),
    ('e+', 'IntermediateElectron'),
    ('mu+', 'Muon'),
    ('mu+', 'IntermediateMuon'),
    ('K+', 'Kaon'),
    ('pi+', 'SlowPion'),
    ('pi+', 'FastPion'),
    ('Lambda0', 'Lambda'), ]
# SignalFraction to calculate probability, -2 if training signal/background ratio should be used
signalFraction = -2

for (symbol, category) in trackLevelParticles:
    particleList = symbol + ':ROE'
    methodPrefix_TL = 'TMVA_' + category + '_TL'
    targetVariable = 'IsFromB(' + category + ')'

# Neurobayes method
    methods = [('NeuroBayes', 'Plugin', '!H:!V:CreateMVAPdfs:NbinsMVAPdf=100:NtrainingIter=20:Preprocessing=122:ShapeTreat=DIAG:TrainingMethod=BFGS:NBIndiPreproFlagByVarname=' + '=34,'.join([Belle2.Variable.makeROOTCompatible(s) for s in variables[category]]) + '=34'), ]

    # Select particles in ROE for different categories of flavour tagging.
    if symbol != 'Lambda0':
        selectParticle(particleList, 'isInRestOfEvent > 0.5', path=roe_path)

    # Check if there is K short in this event
    if symbol == 'K+':
        applyCuts('K+:ROE', '0.1<Kid', path=roe_path)  # Precut done to prevent from overtraining, might be redundant
        selectParticle('pi+:inKaonRoe', 'isInRestOfEvent > 0.5', path=roe_path)
        reconstructDecay('K_S0:ROEKaon -> pi+:inKaonRoe pi-:inKaonRoe',
                         '0.40<=M<=0.60', 1, path=roe_path)
        fitVertex('K_S0:ROEKaon', 0.01, fitter='kfitter', path=roe_path)
        # modify confidence level?!

    if symbol == 'Lambda0':
        selectParticle('pi+:inLambdaRoe', 'isInRestOfEvent > 0.5',
                       path=roe_path)
        selectParticle('p+:inLambdaRoe', 'isInRestOfEvent > 0.5',
                       path=roe_path)
        reconstructDecay('Lambda0:ROE -> pi-:inLambdaRoe p+:inLambdaRoe',
                         '1.00<=M<=1.23', 1, path=roe_path)
        reconstructDecay('K_S0:ROELambda -> pi+:inLambdaRoe pi-:inLambdaRoe',
                         '0.40<=M<=0.60', 1, path=roe_path)
        fitVertex('Lambda0:ROE', 0.01, fitter='kfitter', path=roe_path)
        matchMCTruth('Lambda0:ROE', path=roe_path)
        fitVertex('K_S0:ROELambda', 0.01, fitter='kfitter', path=roe_path)
        # printList('Lambda0:ROE', full=True, path=roe_path)

    if not isTMVAMethodAvailable(workingDirectory + '/' + methodPrefix_TL):
        #print isTMVAMethodAvailable(workingDirectory + '/' + methodPrefix_TL)
        # print workingDirectory + '/' + methodPrefix_TL
        B2INFO('Train TMVAMethod on track level ' + category)
        trainTMVAMethod(
            particleList,
            variables=variables[category],
            target=targetVariable,
            methods=methods,
            prefix=methodPrefix_TL,
            workingDirectory=workingDirectory,
            path=roe_path,)
        trackLevelReady = False
    else:
        B2INFO('Apply TMVAMethod on track level ' + category)
        applyTMVAMethod(
            particleList,
            prefix=methodPrefix_TL,
            signalProbabilityName=targetVariable,
            method=methods[0][0],
            signalFraction=signalFraction,
            workingDirectory=workingDirectory,
            path=roe_path,)

eventLevelReady = trackLevelReady

# Eventlevel -> calculation only on targettrack
eventLevelParticles = trackLevelParticles

if eventLevelReady:
    B2INFO('EVENT LEVEL')
    for (symbol, category) in eventLevelParticles:
        particleList = symbol + ':ROE'
        methodPrefix_EL = 'TMVA_' + category + '_EL'
        # Using MetaVariable
        targetVariable = 'IsRightClass(' + category + ')'
        # VariablesToNtuple to investigate the list before best candidate selection
        # variablesToNTuple(particleList, ["charge", "getExtraInfo(IsFromB(" + category + "))", "qr_Combined"], filename= workingDirectory + "without_best_candidate_selection_" + category + ".root", path=roe_path)
        # Selecting particle with highest r
        applyCuts(particleList, 'hasHighestProbInCat(' + particleList + ',' + 'IsFromB(' + category + ')) > 0.5', path=roe_path)

        if not isTMVAMethodAvailable(workingDirectory + '/' + methodPrefix_EL):
            B2INFO('Train TMVAMethod on event level ' + category)
            trainTMVAMethod(
                particleList,
                variables=variables[category],
                target=targetVariable,
                prefix=methodPrefix_EL,
                methods=methods,
                workingDirectory=workingDirectory,
                path=roe_path,)
            eventLevelReady = False
        else:
            B2INFO('Apply TMVAMethod on event level ' + category)
            applyTMVAMethod(
                particleList,
                prefix=methodPrefix_EL,
                signalProbabilityName=targetVariable,
                method=methods[0][0],
                signalFraction=signalFraction,
                workingDirectory=workingDirectory,
                path=roe_path,)


class RemoveExtraInfoModule(Module):

    """
    Removes the variables added as Extrainfo to the track level particles
    """

    def event(self):
        for (symbol, _) in trackLevelParticles:
            plist = Belle2.PyStoreObj(symbol + ':ROE')
            for i in range(0, plist.obj().getListSize()):
                particle = plist.obj().getParticle(i)
                particle.removeExtraInfo()


combinerLevelReady = eventLevelReady

if combinerLevelReady:
    B2INFO('COMBINER LEVEL')

    variables = [
        'QrOf(e+:ROE, IsRightClass(Electron), IsFromB(Electron))',
        'QrOf(e+:ROE, IsRightClass(IntermediateElectron), IsFromB(IntermediateElectron))',
        'QrOf(mu+:ROE, IsRightClass(Muon), IsFromB(Muon))',
        'QrOf(mu+:ROE, IsRightClass(IntermediateMuon), IsFromB(IntermediateMuon))',
        'QrOf(K+:ROE, IsRightClass(Kaon), IsFromB(Kaon))',
        'QrOf(pi+:ROE, IsRightClass(SlowPion), IsFromB(SlowPion))',
        'QrOf(pi+:ROE, IsRightClass(FastPion), IsFromB(FastPion))',
        'QrOf(Lambda0:ROE, IsRightClass(Lambda), IsFromB(Lambda))',
    ]

#    method_Combiner = [('FastBDT', 'Plugin',
#                       'CreateMVAPdfs:NbinsMVAPdf=100:!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3')]

# Neurobayes method
    method_Combiner = [('NeuroBayes', 'Plugin', '!H:!V:CreateMVAPdfs:NbinsMVAPdf=100:NtrainingIter=20:Preprocessing=122:ShapeTreat=DIAG:TrainingMethod=BFGS:NBIndiPreproFlagByVarname=' + '=34,'.join([Belle2.Variable.makeROOTCompatible(s) for s in variables]) + '=34')]

    if not isTMVAMethodAvailable(workingDirectory + '/' + 'B0Tagger_ROECUT'):
        B2INFO('Train TMVAMethod on combiner level')
        trainTMVAMethod(
            [],
            variables=variables,
            target='qr_Combined',
            prefix='B0Tagger_ROECUT',
            methods=method_Combiner,
            workingDirectory=workingDirectory,
            path=roe_path,
        )
        combinerLevelReady = False
    else:
        B2INFO('Apply TMVAMethod on combiner level')
        applyTMVAMethod(
            [],
            signalProbabilityName='qr_Combined',
            prefix='B0Tagger_ROECUT',
            # can be removed?
            signalClass=1,
            method=method_Combiner[0][0],
            signalFraction=signalFraction,
            workingDirectory=workingDirectory,
            path=roe_path,
        )
        B2INFO(' ')
if not combinerLevelReady:
    roe_path.add_module(RemoveExtraInfoModule())  # Only remove extra info

if combinerLevelReady:

    class MoveTaggerInformationToBExtraInfoModule(Module):

        """
        Adds the flavor tagging information (q*r) from the MC and from the Combiner as ExtraInfo to
        the reconstructed B0 particle
        """

        def event(self):
            roe = Belle2.PyStoreObj('RestOfEvent')
            info = Belle2.PyStoreObj('EventExtraInfo')
            someParticle = Belle2.Particle(None)
            B0_prob = info.obj().getExtraInfo('qr_Combined')
            B0bar_prob = 1 - info.obj().getExtraInfo('qr_Combined')
            qr_Combined = 2 * (info.obj().getExtraInfo('qr_Combined') - 0.5)
            qr_MC = 2 * (mc_variables.variables.evaluate('qr_Combined',
                         someParticle) - 0.5)
            B0 = roe.obj().getRelated('Particles')
            B0.addExtraInfo('B0_prob', B0_prob)
            B0.addExtraInfo('B0bar_prob', B0bar_prob)
            B0.addExtraInfo('qr_Combined', qr_Combined)
            B0.addExtraInfo('qr_MC', qr_MC)
            info.obj().removeExtraInfo()

    roe_path.add_module(MoveTaggerInformationToBExtraInfoModule())  # Move and remove extraInfo
    roe_path.add_module(RemoveExtraInfoModule())


main.for_each('RestOfEvent', 'RestOfEvents', roe_path)

if combinerLevelReady:
    variablesToNTuple('B0', ['getExtraInfo(qr_Combined)', 'getExtraInfo(qr_MC)', 'getExtraInfo(B0_prob)',
                             'getExtraInfo(B0bar_prob)', 'McFlavorOfTagSide', 'mcPDG'], 'TaggingInformation',
                      workingDirectory + '/B0_B0bar_final.root', path=main)


main.add_module(register_module('ProgressBar'))
process(main)
print statistics

B2INFO('')
B2INFO("Training completed. Run 'show_tmva_results TMVA_1_vs_0.root' to view detailed information about the trained methods.")
