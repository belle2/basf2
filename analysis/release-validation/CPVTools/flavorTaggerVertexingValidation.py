#!/usr/bin/env python
# -*- coding: utf-8 -*-

#######################################################
#
# This script produces nTuple files using input mdst files
# to validate the flavor tagger and the vertex
# reconstrcution of B0 mesons.
# The Rest Of Event of the following decay chain is built:
#
# B0 -> J/psi Ks
#        |    |
#        |    +-> pi+ pi-
#        |
#        +-> mu+ mu-
#
# The procedure for the benchmark channel
# B0 -> nu_tau anti-nu_tau
# is also defined.
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
decayChannelTrainedOn = str(sys.argv[3])  # Decay channel of the weight files "JPsiKs" or "nunubar"
decayChannel = str(sys.argv[4])  # Decay channel that will be reconstructed "JPsiKs" or "nunubar"
MCtype = str(sys.argv[5])  # BGx0 for background free MC otherwise BGx1, BelleDataConv for Converted Belle Data
fileNumber = str(sys.argv[6])  # A file number (when sampling in parallel)
workingDirectory = str(sys.argv[7])  # Place where the training samples and the weight files are saved
savingDirectory = str(sys.argv[8])  # Place where the analyzed files are saved
doVertex = str(sys.argv[9])  # Reconstruct B vertices True or False

belleData = ''
if len(sys.argv) > 10:
    if sys.argv[10] != "BelleDataConv":
        B2FATAL("argv[10] only for Belle Data in Expert mode")
    elif sys.argv[10] == "BelleDataConv" and belleOrBelle2Flag != "Belle" and mode != "Expert":
        B2FATAL("argv[10]=BelleDataConv only for Belle Data in Expert mode")
    else:
        belleData = sys.argv[10]

if belleOrBelle2Flag == "Belle":
    from b2biiConversion import convertBelleMdstToBelleIIMdst, setupB2BIIDatabase, setupBelleMagneticField


if decayChannelTrainedOn == 'JPsiKs':
    decayChannelTrainedOn = 'JpsiKs_mu'

# workingDirectory = '.'

# create path
cp_val_path = b2.Path()


def setEnvironment(belleOrBelle2Flag="Belle2"):
    """
    Sets the environment to analyse the mdst files for validation.'

    @param belleOrBelle2Flag Default is 'Belle2' but 'Belle' is possible.
    """

    environmentType = "default"

    if belleOrBelle2Flag == "Belle":
        isBelleMC = True
        if belleData == "BelleDataConv":
            isBelleMC = False
        setupB2BIIDatabase(isBelleMC)  # False for Belle Data True for Belle MC
        os.environ['BELLE_POSTGRES_SERVER'] = 'can51'
        os.environ['USE_GRAND_REPROCESS_DATA'] = '1'

        environmentType = "Belle"

    ma.inputMdstList(environmentType=environmentType, filelist=[], path=cp_val_path)


def reconstructB2JpsiKs_mu(belleOrBelle2Flag='Belle2'):
    """
    Defines the reconstruction procedure for the benchmark channel 'B0 -> J/psi  K_S0'

    @param belleOrBelle2Flag Default is 'Belle2' but 'Belle' is possible.
    """

    # reconstruct J/psi -> mu+ mu- decay using standard muon list and perform MC association
    ma.fillParticleList(decayString='mu+:all', cut='', path=cp_val_path)
    ma.reconstructDecay(decayString='J/psi:mumu -> mu+:all mu-:all', cut='dM<0.11', path=cp_val_path)
    ma.matchMCTruth(list_name='J/psi:mumu', path=cp_val_path)

    if belleOrBelle2Flag == "Belle":

        # use the existent K_S0:mdst list
        ma.matchMCTruth(list_name='K_S0:mdst', path=cp_val_path)

        # reconstruct B0 -> J/psi Ks decay
        ma.reconstructDecay(decayString='B0:sig -> J/psi:mumu  K_S0:mdst', cut='Mbc > 5.2 and abs(deltaE)<0.15', path=cp_val_path)

    if belleOrBelle2Flag == "Belle2":

        # reconstruct Ks from standard pi+ particle list
        ma.fillParticleList(decayString='pi+:all', cut='', path=cp_val_path)
        ma.reconstructDecay(decayString='K_S0:pipi -> pi+:all pi-:all', cut='dM<0.25', path=cp_val_path)

        # reconstruct B0 -> J/psi Ks decay
        ma.reconstructDecay(decayString='B0:sig -> J/psi:mumu K_S0:pipi', cut='Mbc > 5.2 and abs(deltaE)<0.15', path=cp_val_path)


def reconstructB2nunubar():
    """
    Defines the procedure to create a B0 list for the benchmark channel 'B0 -> nu_tau anti-nu_tau'
    """

    ma.findMCDecay(list_name='B0:sig', decay='B0 -> nu_tau anti-nu_tau', writeOut=True, path=cp_val_path)


def applyCPVTools(mode='Expert'):
    """
    Defines the procedure to use the flavor tagger and tagV on the signal 'B0:sig' list.
    It saves also all variables to nTuples needed for evaluation of the performance

    @param mode Default is 'Expert' but also needed for 'Sampler' mode.
    """

    # perform MC matching (MC truth asociation). Always before TagV
    ma.matchMCTruth(list_name='B0:sig', path=cp_val_path)

    # build the rest of the event associated to the B0
    ma.buildRestOfEvent(target_list_name='B0:sig', inputParticlelists=[], path=cp_val_path)

    if mode == 'Sampler':

        ft.flavorTagger(
            particleLists=['B0:sig'],
            mode=mode,
            weightFiles='B2' + decayChannelTrainedOn + MCtype,
            combinerMethods=['TMVA-FBDT', 'FANN-MLP'],
            belleOrBelle2=belleOrBelle2Flag,
            workingDirectory=workingDirectory,
            samplerFileId=str(fileNumber),
            path=cp_val_path)

    if mode == 'Expert':

        ft.flavorTagger(
            particleLists=['B0:sig'],
            combinerMethods=['TMVA-FBDT', 'FANN-MLP'],
            weightFiles='B2' + decayChannelTrainedOn + MCtype,
            belleOrBelle2=belleOrBelle2Flag,
            downloadFromDatabaseIfNotfound=True,
            workingDirectory=workingDirectory,
            samplerFileId=str(fileNumber),
            path=cp_val_path)

    if doVertex == 'True' or mode == 'Expert':

        if doVertex == 'True':
            if decayChannel == "JPsiKs":
                vx.vertexRave(list_name='B0:sig', conf_level=0.0, decay_string='B0:sig -> [J/psi:mumu -> ^mu+ ^mu-] K_S0',
                              constraint='', path=cp_val_path)
            vx.TagV(list_name='B0:sig', MCassociation='breco', path=cp_val_path)
            print("TagV will be used")

    # Select variables that will be stored to ntuple
    fs_vars = vc.pid + vc.track + vc.track_hits + vc.mc_truth
    jpsiandk0s_vars = vc.mc_truth
    vertex_vars = vc.vertex + vc.mc_vertex + vc.kinematics + vc.mc_kinematics
    bvars = vc.reco_stats + \
        vc.deltae_mbc + \
        vc.mc_truth + \
        vc.roe_multiplicities + \
        vc.flavor_tagging + \
        vc.tag_vertex + \
        vc.mc_tag_vertex + \
        vertex_vars

    if decayChannel == "JPsiKs":
        bvars = bvars + \
            vu.create_aliases_for_selected(list_of_variables=fs_vars,
                                           decay_string='B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]') + \
            vu.create_aliases_for_selected(list_of_variables=jpsiandk0s_vars,
                                           decay_string='B0 -> [^J/psi -> mu+ mu-] [^K_S0 -> pi+ pi-]') + \
            vu.create_aliases_for_selected(list_of_variables=vertex_vars,
                                           decay_string='B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]')

    # Saving variables to ntuple
    ma.variablesToNtuple(decayString='B0:sig',
                         variables=bvars,
                         filename=savingDirectory + '/' + 'B2A801-FlavorTagger' +
                         mode + str(fileNumber) + belleOrBelle2Flag + MCtype + belleData + '.root',
                         treename='B0tree',
                         path=cp_val_path)

    ma.summaryOfLists(particleLists=['B0:sig'], path=cp_val_path)


if mode == "Sampler" or mode == "Expert":

    setEnvironment(belleOrBelle2Flag=belleOrBelle2Flag)

    if decayChannel == "nunubar":
        reconstructB2nunubar()

    elif decayChannel == "JPsiKs":
        reconstructB2JpsiKs_mu(belleOrBelle2Flag=belleOrBelle2Flag)

    applyCPVTools(mode=mode)
    ma.process(cp_val_path)
    print(b2.statistics)

if mode == "Teacher":

    ft.flavorTagger(
        particleLists=[],
        mode='Teacher',
        weightFiles='B2' + decayChannelTrainedOn + MCtype,
        combinerMethods=['TMVA-FBDT', 'FANN-MLP'],
        belleOrBelle2=belleOrBelle2Flag,
        downloadFromDatabaseIfNotfound=True,
        uploadToDatabaseAfterTraining=True,
        workingDirectory=workingDirectory,
        path=cp_val_path)
