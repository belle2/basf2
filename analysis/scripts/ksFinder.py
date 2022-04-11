#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import B2INFO, B2FATAL
import basf2
import modularAnalysis as ma
import variables
from variables import utils
import os


def getInputVariablesList():
    # set variables for train/test
    inputVariablesList = [
        'cosVertexMomentum',
        'flightDistance',
        'significanceOfDistance',
        'cosHelicityAngleMomentum',
        'ImpactXY',
        'decayAngle(0)',
        'decayAngle(1)',
        # 'x','y','z',
        # 'px','py','pz',
        'daughterAngleDiffInMother',
        'daughtersDeltaZ',
        'pip_nSVDHits', 'pip_nPXDHits',
        'pin_nSVDHits', 'pin_nPXDHits',
        'pip_dr', 'pin_dr',
        'pip_protonID', 'pin_protonID',
        'M_lambda_p', 'M_lambda_antip',
        'pip_p', 'pin_p',
        'pip_cosTheta', 'pin_cosTheta',
    ]
    return inputVariablesList


ks_finder_info = getInputVariablesList()
utils.add_collection(ks_finder_info, 'ks_finder_info')


def add_default_ks_finder_aliases():
    """
    This function is used to set up variables aliases for ks finder variables
    and define the variable collections.
    """
    B2INFO('KsFinder: creating variables alias.')
    variables.variables.addAlias('M_lambda_p', 'useAlternativeDaughterHypothesis(M, 0:p+)')
    variables.variables.addAlias('M_lambda_antip', 'useAlternativeDaughterHypothesis(M, 1:anti-p-)')
    variables.variables.addAlias('daughtersDeltaZ', 'daughterDiffOf(0, 1, dz)')
    variables.variables.addAlias('cosVertexMomentum', 'cosAngleBetweenMomentumAndVertexVector')
    variables.variables.addAlias('pip_nPXDHits', 'daughter(0,nPXDHits)')
    variables.variables.addAlias('pin_nPXDHits', 'daughter(1,nPXDHits)')
    variables.variables.addAlias('pip_nSVDHits', 'daughter(0,nSVDHits)')
    variables.variables.addAlias('pin_nSVDHits', 'daughter(1,nSVDHits)')
    variables.variables.addAlias('daughterAngleDiffInMother', 'useRestFrame(daughterAngle(0, 1))')
    # variables.variables.addAlias('daughterMomentumDiffInMother', 'useRestFrame(daughterDiffOf(0,1,p))')
    variables.variables.addAlias('pip_p', 'daughter(0,p)')
    variables.variables.addAlias('pin_p', 'daughter(1,p)')
    variables.variables.addAlias('pip_dr', 'daughter(0,dr)')
    variables.variables.addAlias('pin_dr', 'daughter(1,dr)')
    variables.variables.addAlias('pip_cosTheta', 'daughter(0,cosTheta)')
    variables.variables.addAlias('pin_cosTheta', 'daughter(1,cosTheta)')
    variables.variables.addAlias('pip_protonID', 'daughter(0,protonID)')
    variables.variables.addAlias('pin_protonID', 'daughter(1,protonID)')


# ****************************************
# KS FINDER MAIN FUNCTION
# ****************************************


def ksFinder(
    particleListName='',
    listtype='all',
    extraInfoName_V0Selector='KsFinder_V0Selector',
    extraInfoName_LambdaVeto='KsFinder_LambdaVeto',
    useCentralDB=True,
    localDB='',
    path=None
):
    """
    Defines the configuration of Ks Finder process for each Ks candidates
    of the input particle list.

    @param particleLists                reconstructed Ks list with 2 charged daughters
    @param listtype                     Type of Ks cut. When 'all'(default), no cut is applied on Ks.
                                        When 'standard', 'tight', or 'loose', a cut with Ks efficiency
                                        90%, 95%, and 85% is applied.
    @param extraInfoName_V0Selector     Variable name for V0Selector MVA output
    @param extraInfoName_LambdaVeto     Variable name for LambdaVeto MVA output
    @param useCentralDB                 Flag whether weight file is taken from globaltag or local.
                                        If false, weight file is taken from local file specified by localDB.
    @param localDB                      Path for local weight file for MVA.
                                        Only valid when useCentralDB == False.
    @param path                         Basf2 path to execute
    """
    centralDB = "KsFinder_dev"
    basf2.conditions.prepend_globaltag(centralDB)
    add_default_ks_finder_aliases()
    identifier_Ks = 'sugiura_KsFinder_V0Selector',
    identifier_vLambda = 'sugiura_KsFinder_LambdaVeto',
    useLocalDB = not useCentralDB

    # ****************************************
    # Do the Ks classification
    # ****************************************
    particleList = [particleListName]

    if useLocalDB and useCentralDB is False:
        if os.path.exists(localDB):
            B2INFO('KsFinder: use local database:'+localDB)
            basf2.conditions.append_testing_payloads(localDB)
        else:
            B2FATAL('KsFinder: No local database is found.')
    elif useLocalDB is False and useCentralDB:
        B2INFO('KsFinder: use extra central database:'+centralDB)
    else:
        B2FATAL('KsFinder: please specify to use weight file from only local or only central database.')

    path.add_module('MVAMultipleExperts',
                    listNames=particleList,
                    extraInfoNames=[extraInfoName_V0Selector, extraInfoName_LambdaVeto],
                    identifier=[identifier_Ks, identifier_vLambda])

    _effnames = ['all', 'standard', 'tight', 'loose']
    if listtype not in _effnames:
        B2INFO('Invalid List type! No cut is applied on '+particleListName)
    elif listtype == 'all':
        B2INFO('No cut is applied on '+particleListName)
    else:
        V0_thr = 0
        Lambda_thr = 0
        if listtype == 'standard':
            B2INFO('Standard Cut is applied on '+particleListName+'!')
            B2INFO('Threshold is (0.90, 0.11)')
            V0_thr = 0.90
            Lambda_thr = 0.11
        elif listtype == 'tight':
            B2INFO('Tight Cut is applied on '+particleListName+'!')
            B2INFO('Threshold is (0.96, 0.27)')
            V0_thr = 0.96
            Lambda_thr = 0.27
        elif listtype == 'loose':
            B2INFO('Loose Cut is applied on '+particleListName+'!')
            B2INFO('Threshold is (0.49, 0.02)')
            V0_thr = 0.49
            Lambda_thr = 0.02
        cut_string = 'extraInfo('+extraInfoName_V0Selector+')>'+str(V0_thr) + \
            ' and extraInfo('+extraInfoName_LambdaVeto+')>'+str(Lambda_thr)
        ma.applyCuts(list_name=particleListName,
                     cut=cut_string,
                     path=path)
