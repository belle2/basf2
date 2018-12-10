#!/usr/bin/env python
# -*- coding: utf-8 -*-

#######################################################
#
# This script validates the flavor tagger using
# the Rest Of Event of the following decay chain:
#
# B0 -> J/psi Ks
#        |    |
#        |    +-> pi+ pi-
#        |
#        +-> mu+ mu-
#
# Contributor: F. Abudinen (Dec 2018)
#
######################################################
import ROOT
import sysconfig
ROOT.gROOT.ProcessLine(".include " + sysconfig.get_path("include"))
import basf2 as b2
import modularAnalysis as ma
import flavorTagger as ft
import vertex as vx
import variables.collections as vc
import variables.utils as vu
import sys
import os

belleOrBelle2Flag = str(sys.argv[1])  # "Belle" or "Belle2"
mode = str(sys.argv[2])  # "Sampler", "Teacher" or "Expert"
decayChannel = str(sys.argv[3])  # "JPsiKs" or "nunubar"
MCtype = str(sys.argv[4])  # BGx0 for background free MC otherwise BGx1, BelleDataConv for Converted Belle Data
fileNumber = str(sys.argv[5])  # A file number (when sampling in parallel)
workingDirectory = str(sys.argv[6])  # Place where the training samples and the weight files are saved
savingDirectory = str(sys.argv[7])  # Place where the analyzed files are saved
doVertex = str(sys.argv[8])  # Reconstruct B vertices True or False

belleData = ''
if len(sys.argv) > 9:
    if sys.argv[9] != "BelleDataConv":
        B2FATAL("argv[9] only for Belle Data in Expert mode")
    elif sys.argv[9] == "BelleDataConv" and belleOrBelle2Flag != "Belle" and mode != "Expert":
        B2FATAL("argv[9]=BelleDataConv only for Belle Data in Expert mode")
    else:
        belleData = sys.argv[9]

if belleOrBelle2Flag == "Belle":
    from b2biiConversion import convertBelleMdstToBelleIIMdst, setupB2BIIDatabase, setupBelleMagneticField


# workingDirectory = '.'

# create path
ft_val_path = b2.create_path()


def setEnvironment(belleOrBelle2Flag="Belle2"):

    environmentType = "default"

    if belleOrBelle2Flag == "Belle":
        isBelleMC = True
        if belleData == "BelleDataConv":
            isBelleMC = False
        setupB2BIIDatabase(isBelleMC)  # False for Belle Data True for Belle MC
        os.environ['BELLE_POSTGRES_SERVER'] = 'can51'
        os.environ['USE_GRAND_REPROCESS_DATA'] = '1'

        # roinput = b2.register_module('RootInput')
        # ft_val_path.add_module(roinput)
        # progress = b2.register_module('ProgressBar')
        # ft_val_path.add_module(progress)
        #
        # ft_val_path.add_module('Gearbox')
        # ft_val_path.add_module('Geometry', ignoreIfPresent=False, components=['MagneticFieldConstantBelle'])

        environmentType = "Belle"

    ma.inputMdstList(environmentType=environmentType, filelist=[], path=ft_val_path)


def reconstructB2JpsiKs_mu(belleOrBelle2Flag="Belle2"):

    # reconstruct J/psi -> mu+ mu- decay using standard muon list and perform MC association
    ma.fillParticleList(decayString='mu+:all', cut='', path=ft_val_path)
    ma.reconstructDecay(decayString='J/psi:mumu -> mu+:all mu-:all', cut='dM<0.11', path=ft_val_path)
    ma.matchMCTruth(list_name='J/psi:mumu', path=ft_val_path)

    if belleOrBelle2Flag == "Belle":

        # use the existent K_S0:mdst list
        ma.matchMCTruth(list_name='K_S0:mdst', path=ft_val_path)

        # reconstruct B0 -> J/psi Ks decay
        ma.reconstructDecay(decayString='B0:sig -> J/psi:mumu  K_S0:mdst', cut='Mbc > 5.2 and abs(deltaE)<0.15', path=ft_val_path)

    if belleOrBelle2Flag == "Belle2":

        # reconstruct Ks from standard pi+ particle list
        ma.fillParticleList(decayString='pi+:all', cut='', path=ft_val_path)
        ma.reconstructDecay(decayString='K_S0:pipi -> pi+:all pi-:all', cut='dM<0.25', path=ft_val_path)

        # reconstruct B0 -> J/psi Ks decay
        ma.reconstructDecay(decayString='B0:sig -> J/psi:mumu K_S0:pipi', cut='Mbc > 5.2 and abs(deltaE)<0.15', path=ft_val_path)


def reconstructB2nunubar():

    ma.findMCDecay(list_name='B0:sig', decay='B0 -> nu_tau anti-nu_tau', writeOut=True, path=ft_val_path)


def applyCPVTools(mode='Expert'):

    # perform MC matching (MC truth asociation). Always before TagV
    ma.matchMCTruth(list_name='B0:sig', path=ft_val_path)

    # build the rest of the event associated to the B0
    ma.buildRestOfEvent(target_list_name='B0:sig', path=ft_val_path)

    if mode == 'Sampler':

        ft.flavorTagger(
            particleLists=['B0:sig'],
            mode=mode,
            weightFiles='B2JpsiKs_mu' + MCtype,
            combinerMethods=['TMVA-FBDT', 'FANN-MLP'],
            belleOrBelle2=belleOrBelle2Flag,
            workingDirectory=workingDirectory,
            samplerFileId=str(fileNumber),
            path=ft_val_path)

    if mode == 'Expert':

        ft.flavorTagger(
            particleLists=['B0:sig'],
            combinerMethods=['TMVA-FBDT', 'FANN-MLP'],
            weightFiles='B2JpsiKs_mu' + MCtype,
            belleOrBelle2=belleOrBelle2Flag,
            downloadFromDatabaseIfNotfound=True,
            workingDirectory=workingDirectory,
            samplerFileId=str(fileNumber),
            path=ft_val_path)

    if doVertex == 'True' or mode == 'Expert':

        # Due to a problem with TagV using Belle converted data within release 8. Release 9 is Ok!
        # if belleOrBelle2Flag == "Belle2":
        if doVertex == 'True':
            vx.vertexRave(list_name='B0:sig', conf_level=0.0, decay_string='B0:sig -> [J/psi:mumu -> ^mu+ ^mu-] K_S0',
                          constraint='', path=ft_val_path)
            vx.TagV(list_name='B0:sig', MCassociation='breco', path=ft_val_path)
            print("TagV will be used")

        toolsDST = ['EventMetaData', '^B0']
        # ['FlavorTagging[TMVA-FBDT, FANN-MLP, qpCategories]', '^B0']
        toolsDST += ['FlavorTagging[TMVA-FBDT, FANN-MLP, qpCategories]', '^B0']
        toolsDST += ['RecoStats', '^B0']
        toolsDST += ['MCVertex', '^B0 ->  [^J/psi -> mu+ mu-] [^K_S0 -> pi+ pi-]']
        toolsDST += ['Vertex', '^B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]']
        toolsDST += ['MCTagVertex', '^B0']
        toolsDST += ['TagVertex', '^B0']
        toolsDST += ['MCDeltaT', '^B0']
        toolsDST += ['DeltaT', '^B0']
        toolsDST += ['DeltaEMbc', '^B0']
        toolsDST += ['InvMass', '^B0 -> [^J/psi -> mu+ mu-] [^K_S0 -> pi+ pi-]']
        toolsDST += ['CustomFloats[isSignal]', '^B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]']
        toolsDST += ['PDGCode', '^B0']
        toolsDST += ['CMSKinematics', '^B0']
        toolsDST += ['MCHierarchy', 'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]']
        toolsDST += ['PID', 'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]']
        toolsDST += ['TrackHits', 'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]']
        toolsDST += ['Track', 'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]']
        toolsDST += ['MCTruth', '^B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]']
        toolsDST += ['ROEMultiplicities', '^B0']
        toolsDST += ['CustomFloats[TagVOBoost]', '^B0']
        toolsDST += ['CustomFloats[TagVmcOBoost]', '^B0']
        toolsDST += ['CustomFloats[TagVLBoost]', '^B0']
        toolsDST += ['CustomFloats[TagVmcLBoost]', '^B0']
        toolsDST += ['CustomFloats[OBoost]', '^B0']
        toolsDST += ['CustomFloats[mcOBoost]', '^B0']
        toolsDST += ['CustomFloats[LBoost]', '^B0']
        toolsDST += ['CustomFloats[mcLBoost]', '^B0']
#          if mode == 'Expert':
#              toolsDST += ['CustomFloats[extraInfo(QpOfKaon)]', '^B0']
#              toolsDST += ['CustomFloats[extraInfo(WeightedQpOfKaon)]', '^B0']
#              toolsDST += ['CustomFloats[extraInfo(QpOfLambda)]', '^B0']
#              toolsDST += ['CustomFloats[extraInfo(WeightedQpOfLambda)]', '^B0']
#              toolsDST += ['CustomFloats[extraInfo(WeightedQpOfElectron)]', '^B0']
#              toolsDST += ['CustomFloats[extraInfo(WeightedQpOfIntermediateElectron)]', '^B0']
#              toolsDST += ['CustomFloats[extraInfo(WeightedQpOfMuon)]', '^B0']
#              toolsDST += ['CustomFloats[extraInfo(WeightedQpOfIntermediateMuon)]', '^B0']
#              toolsDST += ['CustomFloats[extraInfo(WeightedQpOfKinLepton)]', '^B0']
#              toolsDST += ['CustomFloats[extraInfo(WeightedQpOfIntermediateKinLepton)]', '^B0']
#              toolsDST += ['CustomFloats[extraInfo(WeightedQpOfSlowPion)]', '^B0']
#              toolsDST += ['CustomFloats[extraInfo(WeightedQpOfFSC)]', '^B0']
#              toolsDST += ['CustomFloats[extraInfo(WeightedQpOfMaximumPstar)]', '^B0']
#              toolsDST += ['CustomFloats[extraInfo(WeightedQpOfFastHadron)]', '^B0']
#              toolsDST += ['CustomFloats[extraInfo(WeightedQpOfKaonPion)]', '^B0']
        ma.ntupleFile(file_name=savingDirectory + '/' + 'B2A801-FlavorTagger' +
                      mode + str(fileNumber) + belleOrBelle2Flag + MCtype + belleData + '.root',
                      path=ft_val_path)
        ma.ntupleTree(tree_name='B0tree',
                      list_name='B0:sig',
                      tools=toolsDST, path=ft_val_path)

    # Select variables that will be stored to ntuple
    fs_vars = vc.pid + vc.track + vc.track_hits + vc.mc_truth
    jpsiandk0s_vars = vc.mc_truth
    vertex_vars = vc.vertex + vc.mc_vertex + vc.kinematics + vc.mc_kinematics
    bvars = vc.event_meta_data + \
        vc.reco_stats + \
        vc.deltae_mbc + \
        vc.mc_truth + \
        vc.roe_multiplicities + \
        vc.flavor_tagging + \
        vc.tag_vertex + \
        vc.mc_tag_vertex + \
        vu.create_aliases_for_selected(list_of_variables=fs_vars,
                                       decay_string='B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]') + \
        vu.create_aliases_for_selected(list_of_variables=jpsiandk0s_vars,
                                       decay_string='B0 -> [^J/psi -> mu+ mu-] [^K_S0 -> pi+ pi-]') + \
        vu.create_aliases_for_selected(list_of_variables=vertex_vars,
                                       decay_string='^B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]')

    # Saving variables to ntuple
    ma.variablesToNtuple(decayString='B0:sig',
                         variables=bvars,
                         filename=savingDirectory + '/' + 'B2A801-FlavorTagger' +
                         mode + str(fileNumber) + belleOrBelle2Flag + MCtype + belleData + 'vToN.root',
                         treename='B0tree',
                         path=ft_val_path)

    ma.summaryOfLists(particleLists=['B0:sig'], path=ft_val_path)


if mode == "Sampler" or mode == "Expert":

    setEnvironment(belleOrBelle2Flag=belleOrBelle2Flag)

    if decayChannel == "nunubar":
        reconstructB2nunubar()

    elif decayChannel == "JPsiKs":
        reconstructB2JpsiKs_mu(belleOrBelle2Flag=belleOrBelle2Flag)

    applyCPVTools(mode=mode)
    ma.process(ft_val_path)
    print(b2.statistics)

if mode == "Teacher":

    ft.flavorTagger(
        particleLists=[],
        mode='Teacher',
        weightFiles='B2JpsiKs_mu' + MCtype,
        combinerMethods=['TMVA-FBDT', 'FANN-MLP'],
        belleOrBelle2=belleOrBelle2Flag,
        downloadFromDatabaseIfNotfound=True,
        uploadToDatabaseAfterTraining=True,
        workingDirectory=workingDirectory,
        path=ft_val_path)
