#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
#                                                                        #
# This script produces nTuple files using input mdst files               #
# to validate the flavor tagger and the vertex                           #
# reconstruction of B0 mesons.                                           #
# The Rest Of Event of the following decay chain is built:               #
#                                                                        #
# B0 -> J/psi Ks                                                         #
#        |    |                                                          #
#        |    +-> pi+ pi-                                                #
#        |                                                               #
#        +-> mu+ mu-                                                     #
#                                                                        #
# The procedure for the benchmark channel                                #
# B0 -> nu_tau anti-nu_tau                                               #
# is also defined.                                                       #
#                                                                        #
##########################################################################

import basf2 as b2
import modularAnalysis as ma
import flavorTagger as ft
from dft import DeepFlavorTagger
import vertex as vx
import variables.collections as vc
import variables.utils as vu
from ROOT import Belle2
import argparse
import os


def getCommandLineOptions():
    """ Parses the command line options for the CPV validation and returns the corresponding arguments. """

    parser = argparse.ArgumentParser(description='Script for the validation of CPV tools:\
                                     it defines the procedure to sample the training set, train and test the flavor tagger\
                                     as well as to reconstruct the vertices of B0-> J/psi K_S0 and of the tag-side B0\
                                     using the tagV module.\n\
                                     Usage: basf2 -i inputMDST.root flavorTaggerVertexingValidation.py -- -m Sampler -dc JPsiKs')

    parser.add_argument('-bob2', '--belleOrBelle2Flag', dest='belleOrBelle2Flag',
                        type=str, default='Belle2', choices=['Belle', 'Belle2'],
                        help='Choose Belle for converted Belle data or MC, otherwise choose Belle2.')
    parser.add_argument('-m', '--mode', dest='mode', type=str, required=True,
                        choices=['Sampler', 'Teacher', 'Expert'],
                        help='Working mode of the flavor tagger. Choose Sampler, Teacher or Expert.')
    parser.add_argument('-trc', '--decayChannelTrainedOn', dest='decayChannelTrainedOn',
                        type=str, default='nunubar',
                        help='Decay channel of the weight files. Official samples available are JPsiKs or nunubar.')
    parser.add_argument('-dc', '--decayChannel', dest='decayChannel', type=str, required=True,
                        choices=['JPsiKs', 'nunubar'], help='Decay channel that will be reconstructed. Choose JPsiKs or nunubar.')
    parser.add_argument('-mct', '--mcType', dest='mcType', type=str, default='BGx1',
                        choices=['BGx0', 'BGx1'], help='Type of files. Choose BGx0 for background free Belle II MC. \n' +
                        'Otherwise choose BGx1 for BelleII MC with bkg. or for converted Belle data or MC.')
    parser.add_argument('-fn', '--fileNumber', dest='fileNumber', type=str, default='',
                        help='A file number (when sampling in parallel).')
    parser.add_argument('-wd', '--workingDirectory', dest='workingDirectory', type=str, default='.',
                        help='Path where the training samples and the weight files are saved.')
    parser.add_argument('-sd', '--savingDirectory', dest='savingDirectory', type=str, default='.',
                        help='Path where the analyzed output files are saved.')
    parser.add_argument('-dv', '--doVertex', dest='doVertex', type=bool, default=False,
                        help='Reconstruct B vertices True or False')
    parser.add_argument('-bd', '--belleData', dest='belleData', type=str, default='',
                        choices=['', 'BelleDataConv'], help='Choose BelleDataConv only for Belle Data in Expert mode.')
    args = parser.parse_args()

    if args.belleData == "BelleDataConv":
        if args.belleOrBelle2Flag != "Belle":
            b2.B2FATAL("BelleDataConv only for Belle Data.")
        if args.mode != "Expert":
            b2.B2FATAL("BelleDataConv only in Expert mode.")
        if args.mcType != "BGx1":
            b2.B2FATAL("When using BelleDataConv, mcType must be set to BGx1.")

    return args


def setEnvironment(belleOrBelle2Flag="Belle2"):
    """
    Sets the environment to analyse the mdst files for validation.'

    @param belleOrBelle2Flag Default is 'Belle2' but 'Belle' is possible.
    """

    environmentType = "default"

    if belleOrBelle2Flag == "Belle":

        from b2biiConversion import convertBelleMdstToBelleIIMdst

        os.environ['BELLE_POSTGRES_SERVER'] = 'can51'
        os.environ['USE_GRAND_REPROCESS_DATA'] = '1'

        environmentType = "Belle"

        inputFileList = []
        for iFile in Belle2.Environment.Instance().getInputFilesOverride():
            inputFileList.append(str(iFile))

        convertBelleMdstToBelleIIMdst(inputBelleMDSTFile=inputFileList, path=cp_val_path)

    else:
        ma.inputMdstList(environmentType=environmentType, filelist=[], path=cp_val_path)


def reconstructB2JpsiKs_mu(belleOrBelle2Flag='Belle2'):
    """
    Defines the reconstruction procedure for the benchmark channel 'B0 -> J/psi  K_S0'

    @param belleOrBelle2Flag Default is 'Belle2' but 'Belle' is possible.
    """

    # reconstruct J/psi -> mu+ mu- decay using standard muon list and perform MC association
    ma.fillParticleList(decayString='mu+:all', cut='', path=cp_val_path)
    ma.reconstructDecay(decayString='J/psi:mumu -> mu+:all mu-:all', cut='abs(dM) < 0.11', path=cp_val_path)
    ma.matchMCTruth(list_name='J/psi:mumu', path=cp_val_path)

    if belleOrBelle2Flag == "Belle":

        # use the existent K_S0:mdst list
        ma.matchMCTruth(list_name='K_S0:mdst', path=cp_val_path)

        # reconstruct B0 -> J/psi Ks decay
        ma.reconstructDecay(decayString='B0:sig -> J/psi:mumu  K_S0:mdst', cut='Mbc > 5.2 and abs(deltaE) < 0.15', path=cp_val_path)

    if belleOrBelle2Flag == "Belle2":

        # reconstruct Ks from standard pi+ particle list
        ma.fillParticleList(decayString='pi+:all', cut='', path=cp_val_path)
        ma.reconstructDecay(decayString='K_S0:pipi -> pi+:all pi-:all', cut='abs(dM) < 0.25', path=cp_val_path)

        # reconstruct B0 -> J/psi Ks decay
        ma.reconstructDecay(decayString='B0:sig -> J/psi:mumu K_S0:pipi', cut='Mbc > 5.2 and abs(deltaE) < 0.15', path=cp_val_path)


def reconstructB2nunubar():
    """
    Defines the procedure to create a B0 list for the benchmark channel 'B0 -> nu_tau anti-nu_tau'
    """

    ma.reconstructMCDecay('B0:sig -> nu_tau anti-nu_tau', '', writeOut=True, path=cp_val_path)


def mcMatchAndBuildROE(belleOrBelle2Flag='Belle2'):
    """
    Runs the mc matching and creates the rest of event for the signal particle list.'

    @param belleOrBelle2Flag Default is 'Belle2' but 'Belle' is possible.
    """

    # perform MC matching (MC truth asociation). Always before TagV
    ma.matchMCTruth(list_name='B0:sig', path=cp_val_path)

    # build the rest of the event associated to the B0
    if belleOrBelle2Flag == "Belle":
        target_list_name = 'B0:sig'
        ma.fillParticleList('pi+:mdst', '', path=cp_val_path)
        ma.fillParticleList('gamma:mdst', '', path=cp_val_path)
        # ma.fillParticleList('K_L0:mdst', '', path=cp_val_path)
        inputParticlelists = ['pi+:mdst', 'gamma:mdst']
        roeBuilder = b2.register_module('RestOfEventBuilder')
        roeBuilder.set_name('ROEBuilder_' + target_list_name)
        roeBuilder.param('particleList', target_list_name)
        roeBuilder.param('particleListsInput', inputParticlelists)
        cp_val_path.add_module(roeBuilder)

    if belleOrBelle2Flag == "Belle2":
        ma.buildRestOfEvent(target_list_name='B0:sig', inputParticlelists=[], path=cp_val_path)


def applyCPVTools(mode='Expert'):
    """
    Defines the procedure to use the flavor tagger and tagV on the signal 'B0:sig' list.
    It saves also all variables to nTuples needed for evaluation of the performance

    @param mode Default is 'Expert' but also needed for 'Sampler' mode.
    """

    if mode == 'Sampler':

        ft.flavorTagger(
            particleLists=['B0:sig'],
            mode=mode,
            weightFiles='B2' + decayChannelTrainedOn + mcType,
            combinerMethods=['TMVA-FBDT', 'FANN-MLP'],
            useOnlyLocalWeightFiles=True,
            downloadFromDatabaseIfNotFound=False,
            workingDirectory=workingDirectory,
            samplerFileId=str(fileNumber),
            path=cp_val_path)

    if mode == 'Expert':

        ft.flavorTagger(
            particleLists=['B0:sig'],
            combinerMethods=['TMVA-FBDT', 'FANN-MLP'],
            weightFiles='B2' + decayChannelTrainedOn + mcType,
            useOnlyLocalWeightFiles=True,
            downloadFromDatabaseIfNotFound=False,
            workingDirectory=workingDirectory,
            samplerFileId=str(fileNumber),
            path=cp_val_path)

        # # Preliminarily, DNN Identifier has to be set by hand when validating
        # # The standard name should be however
        # This is temporary till the DNN gets retrained.
        dnnIdentifier = "FlavorTagger_" + belleOrBelle2Flag + "_B2nunubarBGx1OptimizedForDataDNN"
        if belleOrBelle2Flag == "Belle":
            dnnIdentifier = "FlavorTagger_" + belleOrBelle2Flag + "_B2nunubarBGx1DNN"
        b2.conditions.append_globaltag("analysis_tools_release-03-02-00")
        DeepFlavorTagger.DeepFlavorTagger('B0:sig',
                                          mode='expert',
                                          working_dir='',
                                          uniqueIdentifier=dnnIdentifier,
                                          path=cp_val_path)

    if doVertex or mode == 'Expert':

        if doVertex:
            if decayChannel == "JPsiKs":
                # vx.raveFit(list_name='B0:sig', conf_level=0.0, decay_string='B0:sig -> [J/psi:mumu -> ^mu+ ^mu-] K_S0',
                #         constraint='', path=cp_val_path)
                vx.treeFit(list_name='B0:sig', conf_level=-2, path=cp_val_path)
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
        if mode == 'Expert':
            bvars += ft.flavor_tagging + ['DNN_qrCombined', 'extraInfo(dnn_output)']
        else:
            vu._variablemanager.addAlias('qrMC', 'isRelatedRestOfEventB0Flavor')
            bvars += ['qrMC']

        # Saving variables to ntuple
        ma.variablesToNtuple(decayString='B0:sig',
                             variables=bvars,
                             filename=savingDirectory + '/' + 'B2A801-FlavorTagger' +
                             mode + str(fileNumber) + belleOrBelle2Flag + mcType + belleData + '.root',
                             treename='B0tree',
                             path=cp_val_path)

    ma.summaryOfLists(particleLists=['B0:sig'], path=cp_val_path)


if __name__ == '__main__':

    args = getCommandLineOptions()

    belleOrBelle2Flag = args.belleOrBelle2Flag
    mode = args.mode
    decayChannelTrainedOn = args.decayChannelTrainedOn
    decayChannel = args.decayChannel
    mcType = args.mcType
    fileNumber = args.fileNumber
    workingDirectory = args.workingDirectory
    savingDirectory = args.savingDirectory
    doVertex = args.doVertex
    belleData = args.belleData

    if decayChannelTrainedOn == 'JPsiKs':
        decayChannelTrainedOn = 'JpsiKs_mu'

    cp_val_path = b2.Path()

    if mode == "Sampler" or mode == "Expert":

        setEnvironment(belleOrBelle2Flag=belleOrBelle2Flag)

        if decayChannel == "nunubar":
            reconstructB2nunubar()

        elif decayChannel == "JPsiKs":
            reconstructB2JpsiKs_mu(belleOrBelle2Flag=belleOrBelle2Flag)

        mcMatchAndBuildROE(belleOrBelle2Flag=belleOrBelle2Flag)
        applyCPVTools(mode=mode)
        b2.process(cp_val_path)
        print(b2.statistics)

    if mode == "Teacher":

        ft.flavorTagger(
            particleLists=[],
            mode='Teacher',
            weightFiles='B2' + decayChannelTrainedOn + mcType,
            combinerMethods=['TMVA-FBDT', 'FANN-MLP'],
            useOnlyLocalWeightFiles=True,
            downloadFromDatabaseIfNotFound=False,
            uploadToDatabaseAfterTraining=True,
            workingDirectory=workingDirectory,
            path=cp_val_path)
