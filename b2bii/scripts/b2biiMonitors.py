#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from modularAnalysis import fillParticleList
from modularAnalysis import copyParticles
from modularAnalysis import copyList
from modularAnalysis import cutAndCopyLists
from modularAnalysis import matchMCTruth

from vertex import kFit


def addBeamParamsConversionMonitors(outputRootFile='b2biiBeamParamsConversionMonitors.root', path=None):
    """
    In each event saves HER, LER, CMS, x-angle to histograms for monitoring purpuses: e.g. to be compared with
    same distributions obtained within BASF.

    @param outputRootFile name of the output ROOT file to which the histograms are saved.
    @param path modules are added to this path
    """

    # register VariablesToHistogram and fill it with monitored variables
    beam2hist = b2.register_module('VariablesToHistogram')
    beam2hist.param('particleList', '')

    # define variables that are monitored and specify
    # the corresponding histogram (#bins, low, high)
    # ('variable_name', number_of_bins, x_low, x_high)
    beam2hist_expno = ('expNum', 100, 0.0, 99.0)
    beam2hist_runno = ('modulo(runNum,1000)', 1000, 0.0, 999.0)
    beam2hist_evtno = ('modulo(evtNum,10000)', 10000, 0.0, 9999.0)

    beam2hist_Eher = ('Eher', 13000, 7.100, 8.400)
    beam2hist_Eler = ('Eler', 6000, 3.100, 3.700)
    beam2hist_Ecms = ('Ecms', 17000, 9.200, 10.900)
    beam2hist_Xang = ('XAngle', 100, 0.01, 0.03)

    beam2hist_ipx = ('IPX', 100, -0.5, 0.5)
    beam2hist_ipy = ('IPY', 100, -0.5, 0.5)
    beam2hist_ipz = ('IPZ', 100, -5.0, 5.0)

    beam2hist_iperr00 = ('IPCov(0,0)', 100, 0.0, 0.000500)
    beam2hist_iperr01 = ('IPCov(0,1)', 100, 0.0, 0.000001)
    beam2hist_iperr02 = ('IPCov(0,2)', 100, 0.0, 0.010000)
    beam2hist_iperr11 = ('IPCov(1,1)', 100, 0.0, 0.000001)
    beam2hist_iperr12 = ('IPCov(1,2)', 100, 0.0, 0.000001)
    beam2hist_iperr22 = ('IPCov(2,2)', 100, 0.0, 0.500000)

    beam2hist.param('variables',
                    [beam2hist_expno,
                     beam2hist_runno,
                     beam2hist_evtno,
                     beam2hist_Eher,
                     beam2hist_Eler,
                     beam2hist_Ecms,
                     beam2hist_Xang,
                     beam2hist_ipx,
                     beam2hist_ipy,
                     beam2hist_ipz,
                     beam2hist_iperr00,
                     beam2hist_iperr01,
                     beam2hist_iperr02,
                     beam2hist_iperr11,
                     beam2hist_iperr12,
                     beam2hist_iperr22])

    beam2hist.param('fileName', outputRootFile)
    path.add_module(beam2hist)


def addKshortConversionMonitors(outputRootFile='b2biiKshortConversionMonitors.root', path=None):
    """
    Creates copies of KShort particles from 'K_S0:mdst' ParticleList and fills them to a new ParticleList called
    'K_S0:b2bii_monitor'. For each KShort candidate several quantities are stored to histograms with 'VariablesToHistogram' module
    for monitoring purpuses: e.g. to be compared with same distributions obtained within BASF.

    @param outputRootFile name of the output ROOT file to which the histograms are saved.
    @param path modules are added to this path
    """
    # copy KShorts from 'K_S0:mdst' list. We don't want to mess with them.
    copyParticles('K_S0:b2bii_monitor', 'K_S0:mdst', False, path)
    kFit('K_S0:b2bii_monitor', -1, path=path)
    matchMCTruth('K_S0:b2bii_monitor', path)

    # register VariablesToHistogram and fill it with monitored variables
    kshorts2hist = b2.register_module('VariablesToHistogram')
    kshorts2hist.param('particleList', 'K_S0:mdst')
    fittdKS2hist = b2.register_module('VariablesToHistogram')
    fittdKS2hist.param('particleList', 'K_S0:b2bii_monitor')

    # define variables that are monitored and specify
    # the corresponding histogram (#bins, low, high)
    # ('variable_name', number_of_bins, x_low, x_high)
    kshorts2hist_mass = ('M', 100, 0.46, 0.53)
    kshorts2hist_px = ('px', 100, -2.5, 2.5)
    kshorts2hist_py = ('py', 100, -2.5, 2.5)
    kshorts2hist_pz = ('pz', 100, -1.5, 3.5)
    kshorts2hist_e = ('E', 100, 0.0, 4.0)
    kshorts2hist_x = ('x', 100, -50.0, 50.0)
    kshorts2hist_y = ('y', 100, -50.0, 50.0)
    kshorts2hist_z = ('z', 100, -50.0, 50.0)

    kshorts2hist_d0px = ('daughter(0,px)', 100, -2.0, 2.0)
    kshorts2hist_d0py = ('daughter(0,py)', 100, -2.0, 2.0)
    kshorts2hist_d0pz = ('daughter(0,pz)', 100, -2.0, 3.0)
    kshorts2hist_d0e = ('daughter(0,E)', 100, 0.0, 3.0)
    kshorts2hist_d0x = ('daughter(0,x)', 100, -50.0, 50.0)
    kshorts2hist_d0y = ('daughter(0,y)', 100, -50.0, 50.0)
    kshorts2hist_d0z = ('daughter(0,z)', 100, -50.0, 50.0)

    kshorts2hist_d1px = ('daughter(1,px)', 100, -2.0, 2.0)
    kshorts2hist_d1py = ('daughter(1,py)', 100, -2.0, 2.0)
    kshorts2hist_d1pz = ('daughter(1,pz)', 100, -2.0, 3.0)
    kshorts2hist_d1e = ('daughter(1,E)', 100, 0.0, 3.0)
    kshorts2hist_d1x = ('daughter(1,x)', 100, -50.0, 50.0)
    kshorts2hist_d1y = ('daughter(1,y)', 100, -50.0, 50.0)
    kshorts2hist_d1z = ('daughter(1,z)', 100, -50.0, 50.0)

    kshorts2hist_gks = ('extraInfo(goodKs)', 120, -1.1, 1.1)
    kshorts2hist_vlike = ('extraInfo(ksnbVLike)', 120, -1.1, 1.1)
    kshorts2hist_nolam = ('extraInfo(ksnbNoLam)', 120, -1.1, 1.1)
    kshorts2hist_stnrd = ('extraInfo(ksnbStandard)', 120, -1.1, 1.1)

    kshorts2hist.param('variables', [kshorts2hist_mass, kshorts2hist_px, kshorts2hist_py, kshorts2hist_pz,
                                     kshorts2hist_e, kshorts2hist_x, kshorts2hist_y, kshorts2hist_z,
                                     kshorts2hist_d0px, kshorts2hist_d0py, kshorts2hist_d0pz, kshorts2hist_d0e,
                                     kshorts2hist_d0x, kshorts2hist_d0y, kshorts2hist_d0z,
                                     kshorts2hist_d1px, kshorts2hist_d1py, kshorts2hist_d1pz, kshorts2hist_d1e,
                                     kshorts2hist_d1x, kshorts2hist_d1y, kshorts2hist_d1z,
                                     kshorts2hist_gks, kshorts2hist_vlike, kshorts2hist_nolam, kshorts2hist_stnrd])

    fittdKS2hist_mass = ('M', 100, 0.46, 0.53)
    fittdKS2hist_px = ('px', 100, -2.5, 2.5)
    fittdKS2hist_py = ('py', 100, -2.5, 2.5)
    fittdKS2hist_pz = ('pz', 100, -1.5, 3.5)
    fittdKS2hist_e = ('E', 100, 0.0, 4.0)
    fittdKS2hist_x = ('x', 100, -50.0, 50.0)
    fittdKS2hist_y = ('y', 100, -50.0, 50.0)
    fittdKS2hist_z = ('z', 100, -50.0, 50.0)

    fittdKS2hist_pvale = ('chiProb', 120, -1.1, 1.1)

    fittdKS2hist_err00 = ('momVertCovM(0,0)', 100, 0.0, 0.00010)
    fittdKS2hist_err01 = ('momVertCovM(0,1)', 100, 0.0, 0.00005)
    fittdKS2hist_err02 = ('momVertCovM(0,2)', 100, 0.0, 0.00005)
    fittdKS2hist_err03 = ('momVertCovM(0,3)', 100, 0.0, 0.00010)
    fittdKS2hist_err04 = ('momVertCovM(0,4)', 100, 0.0, 0.00020)
    fittdKS2hist_err05 = ('momVertCovM(0,5)', 100, 0.0, 0.00050)
    fittdKS2hist_err06 = ('momVertCovM(0,6)', 100, 0.0, 0.00100)
    fittdKS2hist_err11 = ('momVertCovM(1,1)', 100, 0.0, 0.00010)
    fittdKS2hist_err12 = ('momVertCovM(1,2)', 100, 0.0, 0.00005)
    fittdKS2hist_err13 = ('momVertCovM(1,3)', 100, 0.0, 0.00010)
    fittdKS2hist_err14 = ('momVertCovM(1,4)', 100, 0.0, 0.00050)
    fittdKS2hist_err15 = ('momVertCovM(1,5)', 100, 0.0, 0.00020)
    fittdKS2hist_err16 = ('momVertCovM(1,6)', 100, 0.0, 0.00100)
    fittdKS2hist_err22 = ('momVertCovM(2,2)', 100, 0.0, 0.00020)
    fittdKS2hist_err23 = ('momVertCovM(2,3)', 100, 0.0, 0.00020)
    fittdKS2hist_err24 = ('momVertCovM(2,4)', 100, 0.0, 0.00050)
    fittdKS2hist_err25 = ('momVertCovM(2,5)', 100, 0.0, 0.00050)
    fittdKS2hist_err26 = ('momVertCovM(2,6)', 100, 0.0, 0.00005)
    fittdKS2hist_err33 = ('momVertCovM(3,3)', 100, 0.0, 0.00030)
    fittdKS2hist_err34 = ('momVertCovM(3,4)', 100, 0.0, 0.00050)
    fittdKS2hist_err35 = ('momVertCovM(3,5)', 100, 0.0, 0.00050)
    fittdKS2hist_err36 = ('momVertCovM(3,6)', 100, 0.0, 0.00050)
    fittdKS2hist_err44 = ('momVertCovM(4,4)', 100, 0.0, 0.00100)
    fittdKS2hist_err45 = ('momVertCovM(4,5)', 100, 0.0, 0.00100)
    fittdKS2hist_err46 = ('momVertCovM(4,6)', 100, 0.0, 0.00100)
    fittdKS2hist_err55 = ('momVertCovM(5,5)', 100, 0.0, 0.00100)
    fittdKS2hist_err56 = ('momVertCovM(5,6)', 100, 0.0, 0.00100)
    fittdKS2hist_err66 = ('momVertCovM(6,6)', 100, 0.0, 0.00100)

    # mc truth information
    fittdKS2hist_ksd0MCPDG = ('abs(daughter(0,mcPDG))', 250, 0.0, 2499.0)
    fittdKS2hist_ksd1MCPDG = ('abs(daughter(1,mcPDG))', 250, 0.0, 2499.0)
    fittdKS2hist_ksd0MCP = ('daughter(0,mcP)', 100, 0.0, 3.5)
    fittdKS2hist_ksd1MCP = ('daughter(1,mcP)', 100, 0.0, 3.5)

    fittdKS2hist.param('variables', [fittdKS2hist_mass, fittdKS2hist_px, fittdKS2hist_py, fittdKS2hist_pz,
                                     fittdKS2hist_e, fittdKS2hist_x, fittdKS2hist_y, fittdKS2hist_z,
                                     fittdKS2hist_pvale,
                                     fittdKS2hist_err00, fittdKS2hist_err01, fittdKS2hist_err02, fittdKS2hist_err03,
                                     fittdKS2hist_err04, fittdKS2hist_err05, fittdKS2hist_err06,
                                     fittdKS2hist_err11, fittdKS2hist_err12, fittdKS2hist_err13,
                                     fittdKS2hist_err14, fittdKS2hist_err15, fittdKS2hist_err16,
                                     fittdKS2hist_err22, fittdKS2hist_err23, fittdKS2hist_err24,
                                     fittdKS2hist_err25, fittdKS2hist_err26,
                                     fittdKS2hist_err33, fittdKS2hist_err34, fittdKS2hist_err35, fittdKS2hist_err36,
                                     fittdKS2hist_err44, fittdKS2hist_err45, fittdKS2hist_err46,
                                     fittdKS2hist_err55, fittdKS2hist_err56,
                                     fittdKS2hist_err66,
                                     fittdKS2hist_ksd0MCPDG, fittdKS2hist_ksd1MCPDG,
                                     fittdKS2hist_ksd0MCP, fittdKS2hist_ksd1MCP])

    kshorts2hist.param('fileName', outputRootFile)
    outputRootFileAVF = outputRootFile.replace(".root", "-AVF.root")
    fittdKS2hist.param('fileName', outputRootFileAVF)
    path.add_module(kshorts2hist)
    path.add_module(fittdKS2hist)


def addKlongConversionMonitors(outputRootFile='b2biiKlongConversionMonitors.root', path=None):
    """
    Creates copies of KLong particles from 'K_L0:mdst' ParticleList and fills them to a new ParticleList called
    'K_L0:b2bii_monitor'. For each Klong candidate several quantities are stored to histograms with 'VariablesToHistogram' module
    for monitoring purpuses: e.g. to be compared with same distributions obtained within BASF.

    @param outputRootFile name of the output ROOT file to which the histograms are saved.
    @param path modules are added to this path
    """

    matchMCTruth('K_L0:mdst', path)

    # register VariablesToHistogram and fill it with monitored variables
    klong2hist = b2.register_module('VariablesToHistogram')
    klong2hist.param('particleList', 'K_L0:mdst')

    # define variables that are monitored and specify
    # the corresponding histogram (#bins, low, high)
    # ('variable_name', number_of_bins, x_low, x_high)
    klong2hist_pos_x = ('klmClusterPositionX', 100, -3.0, 3.0)
    klong2hist_pos_y = ('klmClusterPositionY', 100, -3.0, 3.0)
    klong2hist_pos_z = ('klmClusterPositionZ', 100, -3.0, 3.0)
    klong2hist_layers = ('klmClusterLayers', 100, 0.0, 35.0)
    klong2hist_innermost_layer = ('klmClusterInnermostLayer', 100, 0.0, 35.0)

    # (vague) mc truth missing
    # mcPDG, mE, mPX, mPY, mPZ
    klong2hist_mcPDG = ('mcPDG', 1000, 0, 1000)
    klong2hist_mE = ('mcE', 100, 0.0, 6.0)
    klong2hist_mPX = ('mcPX', 100, -5.0, 5.0)
    klong2hist_mPY = ('mcPY', 100, -5.0, 5.0)
    klong2hist_mPZ = ('mcPZ', 100, -5.0, 5.0)
    klong2hist.param('variables', [klong2hist_pos_x, klong2hist_pos_y, klong2hist_pos_z,
                                   klong2hist_layers, klong2hist_innermost_layer,
                                   klong2hist_mcPDG, klong2hist_mE, klong2hist_mPX, klong2hist_mPY, klong2hist_mPZ
                                   ])

    klong2hist.param('fileName', outputRootFile)

    path.add_module(klong2hist)


def addLambda0ConversionMonitors(outputRootFile='b2biiLambda0ConversionMonitors.root', path=None):
    """
    Creates copies of Lambda0 particles from 'Lambda0:mdst' ParticleList and c.c. and fills them to a new ParticleList called
    'Lambda0:b2bii_monitor'. For each Lambda0 candidate several quantities are stored to histograms with 'VariablesToHistogram'
    module for monitoring purpuses: e.g. to be compared with same distributions obtained within BASF.

    @param outputRootFile name of the output ROOT file to which the histograms are saved.
    @param path modules are added to this path
    """

    copyParticles('Lambda0:b2bii_monitor', 'Lambda0:mdst', False, path)
    kFit('Lambda0:b2bii_monitor', -1, path=path)
    matchMCTruth('Lambda0:b2bii_monitor', path)

    # register VariablesToHistogram and fill it with monitored variables
    lambdas2hist = b2.register_module('VariablesToHistogram')
    lambdas2hist.param('particleList', 'Lambda0:mdst')
    fittdL02hist = b2.register_module('VariablesToHistogram')
    fittdL02hist.param('particleList', 'Lambda0:b2bii_monitor')

    # define variables that are monitored and specify
    # the corresponding histogram (#bins, low, high)
    # ('variable_name', number_of_bins, x_low, x_high)
    lambdas2hist_mass = ('M', 100, 1.095, 1.135)
    lambdas2hist_px = ('px', 100, -2.5, 2.5)
    lambdas2hist_py = ('py', 100, -2.5, 2.5)
    lambdas2hist_pz = ('pz', 100, -1.5, 3.5)
    lambdas2hist_e = ('E', 100, 0.0, 4.0)
    lambdas2hist_x = ('x', 100, -50.0, 50.0)
    lambdas2hist_y = ('y', 100, -50.0, 50.0)
    lambdas2hist_z = ('z', 100, -50.0, 50.0)

    lambdas2hist_d0px = ('daughter(0,px)', 100, -2.0, 2.0)
    lambdas2hist_d0py = ('daughter(0,py)', 100, -2.0, 2.0)
    lambdas2hist_d0pz = ('daughter(0,pz)', 100, -2.0, 3.0)
    lambdas2hist_d0e = ('daughter(0,E)', 100, 0.0, 3.0)
    lambdas2hist_d0x = ('daughter(0,x)', 100, -50.0, 50.0)
    lambdas2hist_d0y = ('daughter(0,y)', 100, -50.0, 50.0)
    lambdas2hist_d0z = ('daughter(0,z)', 100, -50.0, 50.0)

    lambdas2hist_d1px = ('daughter(1,px)', 100, -2.0, 2.0)
    lambdas2hist_d1py = ('daughter(1,py)', 100, -2.0, 2.0)
    lambdas2hist_d1pz = ('daughter(1,pz)', 100, -2.0, 3.0)
    lambdas2hist_d1e = ('daughter(1,E)', 100, 0.0, 3.0)
    lambdas2hist_d1x = ('daughter(1,x)', 100, -50.0, 50.0)
    lambdas2hist_d1y = ('daughter(1,y)', 100, -50.0, 50.0)
    lambdas2hist_d1z = ('daughter(1,z)', 100, -50.0, 50.0)

    lambdas2hist.param('variables', [lambdas2hist_mass, lambdas2hist_px, lambdas2hist_py, lambdas2hist_pz,
                                     lambdas2hist_e, lambdas2hist_x, lambdas2hist_y, lambdas2hist_z,
                                     lambdas2hist_d0px, lambdas2hist_d0py, lambdas2hist_d0pz, lambdas2hist_d0e,
                                     lambdas2hist_d0x, lambdas2hist_d0y, lambdas2hist_d0z,
                                     lambdas2hist_d1px, lambdas2hist_d1py, lambdas2hist_d1pz, lambdas2hist_d1e,
                                     lambdas2hist_d1x, lambdas2hist_d1y, lambdas2hist_d1z])

    fittdL02hist_mass = ('M', 100, 1.095, 1.135)
    fittdL02hist_px = ('px', 100, -2.5, 2.5)
    fittdL02hist_py = ('py', 100, -2.5, 2.5)
    fittdL02hist_pz = ('pz', 100, -1.5, 3.5)
    fittdL02hist_e = ('E', 100, 0.0, 4.0)
    fittdL02hist_x = ('x', 100, -50.0, 50.0)
    fittdL02hist_y = ('y', 100, -50.0, 50.0)
    fittdL02hist_z = ('z', 100, -50.0, 50.0)

    fittdL02hist_pvale = ('chiProb', 120, -1.1, 1.1)

    fittdL02hist_err00 = ('momVertCovM(0,0)', 100, 0.0, 0.00010)
    fittdL02hist_err01 = ('momVertCovM(0,1)', 100, 0.0, 0.00005)
    fittdL02hist_err02 = ('momVertCovM(0,2)', 100, 0.0, 0.00005)
    fittdL02hist_err03 = ('momVertCovM(0,3)', 100, 0.0, 0.00010)
    fittdL02hist_err04 = ('momVertCovM(0,4)', 100, 0.0, 0.00020)
    fittdL02hist_err05 = ('momVertCovM(0,5)', 100, 0.0, 0.00050)
    fittdL02hist_err06 = ('momVertCovM(0,6)', 100, 0.0, 0.00100)
    fittdL02hist_err11 = ('momVertCovM(1,1)', 100, 0.0, 0.00010)
    fittdL02hist_err12 = ('momVertCovM(1,2)', 100, 0.0, 0.00005)
    fittdL02hist_err13 = ('momVertCovM(1,3)', 100, 0.0, 0.00010)
    fittdL02hist_err14 = ('momVertCovM(1,4)', 100, 0.0, 0.00050)
    fittdL02hist_err15 = ('momVertCovM(1,5)', 100, 0.0, 0.00020)
    fittdL02hist_err16 = ('momVertCovM(1,6)', 100, 0.0, 0.00100)
    fittdL02hist_err22 = ('momVertCovM(2,2)', 100, 0.0, 0.00020)
    fittdL02hist_err23 = ('momVertCovM(2,3)', 100, 0.0, 0.00020)
    fittdL02hist_err24 = ('momVertCovM(2,4)', 100, 0.0, 0.00050)
    fittdL02hist_err25 = ('momVertCovM(2,5)', 100, 0.0, 0.00050)
    fittdL02hist_err26 = ('momVertCovM(2,6)', 100, 0.0, 0.00005)
    fittdL02hist_err33 = ('momVertCovM(3,3)', 100, 0.0, 0.00030)
    fittdL02hist_err34 = ('momVertCovM(3,4)', 100, 0.0, 0.00050)
    fittdL02hist_err35 = ('momVertCovM(3,5)', 100, 0.0, 0.00050)
    fittdL02hist_err36 = ('momVertCovM(3,6)', 100, 0.0, 0.00050)
    fittdL02hist_err44 = ('momVertCovM(4,4)', 100, 0.0, 0.00100)
    fittdL02hist_err45 = ('momVertCovM(4,5)', 100, 0.0, 0.00100)
    fittdL02hist_err46 = ('momVertCovM(4,6)', 100, 0.0, 0.00100)
    fittdL02hist_err55 = ('momVertCovM(5,5)', 100, 0.0, 0.00100)
    fittdL02hist_err56 = ('momVertCovM(5,6)', 100, 0.0, 0.00100)
    fittdL02hist_err66 = ('momVertCovM(6,6)', 100, 0.0, 0.00100)

    # mc truth information
    fittdL02hist_ksd0MCPDG = ('abs(daughter(0,mcPDG))', 250, 0.0, 2499.0)
    fittdL02hist_ksd1MCPDG = ('abs(daughter(1,mcPDG))', 250, 0.0, 2499.0)
    fittdL02hist_ksd0MCP = ('daughter(0,mcP)', 100, 0.0, 3.5)
    fittdL02hist_ksd1MCP = ('daughter(1,mcP)', 100, 0.0, 3.5)

    fittdL02hist.param('variables', [fittdL02hist_mass, fittdL02hist_px, fittdL02hist_py, fittdL02hist_pz,
                                     fittdL02hist_e, fittdL02hist_x, fittdL02hist_y, fittdL02hist_z,
                                     fittdL02hist_pvale,
                                     fittdL02hist_err00, fittdL02hist_err01, fittdL02hist_err02, fittdL02hist_err03,
                                     fittdL02hist_err04, fittdL02hist_err05, fittdL02hist_err06,
                                     fittdL02hist_err11, fittdL02hist_err12, fittdL02hist_err13,
                                     fittdL02hist_err14, fittdL02hist_err15, fittdL02hist_err16,
                                     fittdL02hist_err22, fittdL02hist_err23, fittdL02hist_err24,
                                     fittdL02hist_err25, fittdL02hist_err26,
                                     fittdL02hist_err33, fittdL02hist_err34, fittdL02hist_err35, fittdL02hist_err36,
                                     fittdL02hist_err44, fittdL02hist_err45, fittdL02hist_err46,
                                     fittdL02hist_err55, fittdL02hist_err56,
                                     fittdL02hist_err66,
                                     fittdL02hist_ksd0MCPDG, fittdL02hist_ksd1MCPDG,
                                     fittdL02hist_ksd0MCP, fittdL02hist_ksd1MCP])

    lambdas2hist.param('fileName', outputRootFile)
    outputRootFileAVF = outputRootFile.replace(".root", "-AVF.root")
    fittdL02hist.param('fileName', outputRootFileAVF)
    path.add_module(lambdas2hist)
    path.add_module(fittdL02hist)


def addConvertedPhotonConversionMonitors(outputRootFile='b2biiConvertedPhotonConversionMonitors.root', path=None):
    """
    Creates copies of gamma particles from 'gamma:v0mdst' ParticleList and fills them to a new ParticleList called
    'gamma:v0_b2bii_monitor'. For each gamma candidate several quantities are stored to histograms with 'VariablesToHistogram'
    module for monitoring purpuses: e.g. to be compared with same distributions obtained within BASF.

    @param outputRootFile name of the output ROOT file to which the histograms are saved.
    @param path modules are added to this path
    """

    copyParticles('gamma:v0_b2bii_monitor', 'gamma:v0mdst', False, path)
    kFit('gamma:v0_b2bii_monitor', -1, path=path)
    matchMCTruth('gamma:v0_b2bii_monitor', path)

    # register VariablesTohistogram and fill it with monitored variables
    convgam2hist = b2.register_module('VariablesToHistogram')
    convgam2hist.param('particleList', 'gamma:v0mdst')
    fittdCG2hist = b2.register_module('VariablesToHistogram')
    fittdCG2hist.param('particleList', 'gamma:v0_b2bii_monitor')

    # define variables that are monitored and specify
    # the corresponding histogram (#bins, low, high)
    # ('variable_name', number_of_bins, x_low, x_high)
    convgam2hist_mass = ('M', 100, 0.0, 0.050)
    convgam2hist_px = ('px', 100, -2.5, 2.5)
    convgam2hist_py = ('py', 100, -2.5, 2.5)
    convgam2hist_pz = ('pz', 100, -1.5, 3.5)
    convgam2hist_e = ('E', 100, 0.0, 4.0)
    convgam2hist_x = ('x', 100, -50.0, 50.0)
    convgam2hist_y = ('y', 100, -50.0, 50.0)
    convgam2hist_z = ('z', 100, -50.0, 50.0)

    convgam2hist_d0px = ('daughter(0,px)', 100, -2.0, 2.0)
    convgam2hist_d0py = ('daughter(0,py)', 100, -2.0, 2.0)
    convgam2hist_d0pz = ('daughter(0,pz)', 100, -2.0, 3.0)
    convgam2hist_d0e = ('daughter(0,E)', 100, 0.0, 3.0)
    convgam2hist_d0x = ('daughter(0,x)', 100, -50.0, 50.0)
    convgam2hist_d0y = ('daughter(0,y)', 100, -50.0, 50.0)
    convgam2hist_d0z = ('daughter(0,z)', 100, -50.0, 50.0)

    convgam2hist_d1px = ('daughter(1,px)', 100, -2.0, 2.0)
    convgam2hist_d1py = ('daughter(1,py)', 100, -2.0, 2.0)
    convgam2hist_d1pz = ('daughter(1,pz)', 100, -2.0, 3.0)
    convgam2hist_d1e = ('daughter(1,E)', 100, 0.0, 3.0)
    convgam2hist_d1x = ('daughter(1,x)', 100, -50.0, 50.0)
    convgam2hist_d1y = ('daughter(1,y)', 100, -50.0, 50.0)
    convgam2hist_d1z = ('daughter(1,z)', 100, -50.0, 50.0)

    convgam2hist.param('variables', [convgam2hist_mass, convgam2hist_px, convgam2hist_py, convgam2hist_pz,
                                     convgam2hist_e, convgam2hist_x, convgam2hist_y, convgam2hist_z,
                                     convgam2hist_d0px, convgam2hist_d0py, convgam2hist_d0pz, convgam2hist_d0e,
                                     convgam2hist_d0x, convgam2hist_d0y, convgam2hist_d0z,
                                     convgam2hist_d1px, convgam2hist_d1py, convgam2hist_d1pz, convgam2hist_d1e,
                                     convgam2hist_d1x, convgam2hist_d1y, convgam2hist_d1z])

    fittdCG2hist_mass = ('M', 100, 0.0, 0.050)
    fittdCG2hist_px = ('px', 100, -2.5, 2.5)
    fittdCG2hist_py = ('py', 100, -2.5, 2.5)
    fittdCG2hist_pz = ('pz', 100, -1.5, 3.5)
    fittdCG2hist_e = ('E', 100, 0.0, 4.0)
    fittdCG2hist_x = ('x', 100, -50.0, 50.0)
    fittdCG2hist_y = ('y', 100, -50.0, 50.0)
    fittdCG2hist_z = ('z', 100, -50.0, 50.0)

    fittdCG2hist_pvale = ('chiProb', 120, -1.1, 1.1)

    fittdCG2hist_err00 = ('momVertCovM(0,0)', 100, 0.0, 0.00010)
    fittdCG2hist_err01 = ('momVertCovM(0,1)', 100, 0.0, 0.00005)
    fittdCG2hist_err02 = ('momVertCovM(0,2)', 100, 0.0, 0.00005)
    fittdCG2hist_err03 = ('momVertCovM(0,3)', 100, 0.0, 0.00010)
    fittdCG2hist_err04 = ('momVertCovM(0,4)', 100, 0.0, 0.00020)
    fittdCG2hist_err05 = ('momVertCovM(0,5)', 100, 0.0, 0.00050)
    fittdCG2hist_err06 = ('momVertCovM(0,6)', 100, 0.0, 0.00100)
    fittdCG2hist_err11 = ('momVertCovM(1,1)', 100, 0.0, 0.00010)
    fittdCG2hist_err12 = ('momVertCovM(1,2)', 100, 0.0, 0.00005)
    fittdCG2hist_err13 = ('momVertCovM(1,3)', 100, 0.0, 0.00010)
    fittdCG2hist_err14 = ('momVertCovM(1,4)', 100, 0.0, 0.00050)
    fittdCG2hist_err15 = ('momVertCovM(1,5)', 100, 0.0, 0.00020)
    fittdCG2hist_err16 = ('momVertCovM(1,6)', 100, 0.0, 0.00100)
    fittdCG2hist_err22 = ('momVertCovM(2,2)', 100, 0.0, 0.00020)
    fittdCG2hist_err23 = ('momVertCovM(2,3)', 100, 0.0, 0.00020)
    fittdCG2hist_err24 = ('momVertCovM(2,4)', 100, 0.0, 0.00050)
    fittdCG2hist_err25 = ('momVertCovM(2,5)', 100, 0.0, 0.00050)
    fittdCG2hist_err26 = ('momVertCovM(2,6)', 100, 0.0, 0.00005)
    fittdCG2hist_err33 = ('momVertCovM(3,3)', 100, 0.0, 0.00030)
    fittdCG2hist_err34 = ('momVertCovM(3,4)', 100, 0.0, 0.00050)
    fittdCG2hist_err35 = ('momVertCovM(3,5)', 100, 0.0, 0.00050)
    fittdCG2hist_err36 = ('momVertCovM(3,6)', 100, 0.0, 0.00050)
    fittdCG2hist_err44 = ('momVertCovM(4,4)', 100, 0.0, 0.00100)
    fittdCG2hist_err45 = ('momVertCovM(4,5)', 100, 0.0, 0.00100)
    fittdCG2hist_err46 = ('momVertCovM(4,6)', 100, 0.0, 0.00100)
    fittdCG2hist_err55 = ('momVertCovM(5,5)', 100, 0.0, 0.00100)
    fittdCG2hist_err56 = ('momVertCovM(5,6)', 100, 0.0, 0.00100)
    fittdCG2hist_err66 = ('momVertCovM(6,6)', 100, 0.0, 0.00100)

    # mc truth information
    fittdCG2hist_ksd0MCPDG = ('abs(daughter(0,mcPDG))', 250, 0.0, 2499.0)
    fittdCG2hist_ksd1MCPDG = ('abs(daughter(1,mcPDG))', 250, 0.0, 2499.0)
    fittdCG2hist_ksd0MCP = ('daughter(0,mcP)', 100, 0.0, 3.5)
    fittdCG2hist_ksd1MCP = ('daughter(1,mcP)', 100, 0.0, 3.5)

    fittdCG2hist.param('variables', [fittdCG2hist_mass, fittdCG2hist_px, fittdCG2hist_py, fittdCG2hist_pz,
                                     fittdCG2hist_e, fittdCG2hist_x, fittdCG2hist_y, fittdCG2hist_z,
                                     fittdCG2hist_pvale,
                                     fittdCG2hist_err00, fittdCG2hist_err01, fittdCG2hist_err02, fittdCG2hist_err03,
                                     fittdCG2hist_err04, fittdCG2hist_err05, fittdCG2hist_err06,
                                     fittdCG2hist_err11, fittdCG2hist_err12, fittdCG2hist_err13,
                                     fittdCG2hist_err14, fittdCG2hist_err15, fittdCG2hist_err16,
                                     fittdCG2hist_err22, fittdCG2hist_err23, fittdCG2hist_err24,
                                     fittdCG2hist_err25, fittdCG2hist_err26,
                                     fittdCG2hist_err33, fittdCG2hist_err34, fittdCG2hist_err35, fittdCG2hist_err36,
                                     fittdCG2hist_err44, fittdCG2hist_err45, fittdCG2hist_err46,
                                     fittdCG2hist_err55, fittdCG2hist_err56,
                                     fittdCG2hist_err66,
                                     fittdCG2hist_ksd0MCPDG, fittdCG2hist_ksd1MCPDG,
                                     fittdCG2hist_ksd0MCP, fittdCG2hist_ksd1MCP])

    convgam2hist.param('fileName', outputRootFile)
    outputRootFileAVF = outputRootFile.replace(".root", "-AVF.root")
    fittdCG2hist.param('fileName', outputRootFileAVF)
    path.add_module(convgam2hist)
    path.add_module(fittdCG2hist)


def addTrackConversionMonitors(outputRootFile='b2biiTrackConversionMonitors.root', path=None):
    """
    Creates 'pi+:b2bii_monitor' ParticleLists and fills it with all converted charged tracks as charged pions.
    For each charged pion (track) several quantities are stored to histograms with 'VariablesToHistogram' module
    for monitoring purpuses: e.g. to be compared with same distributions obtained within BASF.

    @param outputRootFile name of the output ROOT file to which the histograms are saved.
    @param path modules are added to this path
    """

    # create charged pions from all charged tracks
    fillParticleList('pi+:b2bii_monitor', '', False, path)

    # register VariablesToHistogram and fill it with monitored variables
    tracks2hist = b2.register_module('VariablesToHistogram')
    tracks2hist.param('particleList', 'pi+:b2bii_monitor')

    # define variables that are monitored and specify
    # the corresponding histogram (#bins, low, high)
    # ('variable_name', number_of_bins, x_low, x_high)
    tracks2hist_KPID = ('atcPIDBelle(3,2)', 110, -0.05, 1.05)
    tracks2hist_PRKID = ('atcPIDBelle(4,3)', 110, -0.05, 1.05)
    tracks2hist_PRPID = ('atcPIDBelle(4,2)', 110, -0.05, 1.05)
    tracks2hist_EID = ('eIDBelle', 110, -0.05, 1.05)
    tracks2hist_MUID = ('muIDBelle', 110, -0.05, 1.05)
    tracks2hist_MUIDQ = ('muIDBelleQuality', 110, -0.05, 1.05)

    tracks2hist_px = ('px', 100, -3.0, 3.0)
    tracks2hist_py = ('py', 100, -3.0, 3.0)
    tracks2hist_pz = ('pz', 100, -3.0, 3.0)
    tracks2hist_E = ('E', 100, 0.0, 4.0)
    tracks2hist_x = ('x', 100, -5.0, 5.0)
    tracks2hist_y = ('y', 100, -5.0, 5.0)
    tracks2hist_z = ('z', 100, -5.0, 5.0)

    tracks2hist_Err00 = ('momVertCovM(0,0)', 100, -0.0001, 0.0001)
    tracks2hist_Err01 = ('momVertCovM(0,1)', 100, -0.0001, 0.0001)
    tracks2hist_Err02 = ('momVertCovM(0,2)', 100, -0.0001, 0.0001)
    tracks2hist_Err03 = ('momVertCovM(0,3)', 100, -0.0001, 0.0001)
    tracks2hist_Err04 = ('momVertCovM(0,4)', 100, -0.0001, 0.0001)
    tracks2hist_Err05 = ('momVertCovM(0,5)', 100, -0.0001, 0.0001)
    tracks2hist_Err06 = ('momVertCovM(0,6)', 100, -0.0001, 0.0001)
    tracks2hist_Err11 = ('momVertCovM(1,1)', 100, -0.0001, 0.0001)
    tracks2hist_Err12 = ('momVertCovM(1,2)', 100, -0.0001, 0.0001)
    tracks2hist_Err13 = ('momVertCovM(1,3)', 100, -0.0001, 0.0001)
    tracks2hist_Err14 = ('momVertCovM(1,4)', 100, -0.0001, 0.0001)
    tracks2hist_Err15 = ('momVertCovM(1,5)', 100, -0.0001, 0.0001)
    tracks2hist_Err16 = ('momVertCovM(1,6)', 100, -0.0001, 0.0001)
    tracks2hist_Err22 = ('momVertCovM(2,2)', 100, -0.0001, 0.0001)
    tracks2hist_Err23 = ('momVertCovM(2,3)', 100, -0.0001, 0.0001)
    tracks2hist_Err24 = ('momVertCovM(2,4)', 100, -0.0001, 0.0001)
    tracks2hist_Err25 = ('momVertCovM(2,5)', 100, -0.0001, 0.0001)
    tracks2hist_Err26 = ('momVertCovM(2,6)', 100, -0.0001, 0.0001)
    tracks2hist_Err33 = ('momVertCovM(3,3)', 100, -0.0001, 0.0001)
    tracks2hist_Err34 = ('momVertCovM(3,4)', 100, -0.0001, 0.0001)
    tracks2hist_Err35 = ('momVertCovM(3,5)', 100, -0.0001, 0.0001)
    tracks2hist_Err36 = ('momVertCovM(3,6)', 100, -0.0001, 0.0001)
    tracks2hist_Err44 = ('momVertCovM(4,4)', 100, -0.0001, 0.0001)
    tracks2hist_Err45 = ('momVertCovM(4,5)', 100, -0.0001, 0.0001)
    tracks2hist_Err46 = ('momVertCovM(4,6)', 100, -0.0001, 0.0001)
    tracks2hist_Err55 = ('momVertCovM(5,5)', 100, -0.0001, 0.0001)
    tracks2hist_Err56 = ('momVertCovM(5,6)', 100, -0.0001, 0.0001)
    tracks2hist_Err66 = ('momVertCovM(6,6)', 100, -0.0001, 0.0001)

    tracks2hist.param('variables',
                      [tracks2hist_KPID, tracks2hist_PRKID, tracks2hist_PRPID,
                       tracks2hist_EID, tracks2hist_MUID, tracks2hist_MUIDQ,

                       tracks2hist_px, tracks2hist_py, tracks2hist_pz, tracks2hist_E,
                       tracks2hist_x, tracks2hist_y, tracks2hist_z,

                       tracks2hist_Err00, tracks2hist_Err01, tracks2hist_Err02, tracks2hist_Err03,
                       tracks2hist_Err04, tracks2hist_Err05, tracks2hist_Err06, tracks2hist_Err11,
                       tracks2hist_Err12, tracks2hist_Err13, tracks2hist_Err14, tracks2hist_Err15,
                       tracks2hist_Err16, tracks2hist_Err22, tracks2hist_Err23, tracks2hist_Err24,
                       tracks2hist_Err25, tracks2hist_Err26, tracks2hist_Err33, tracks2hist_Err34,
                       tracks2hist_Err35, tracks2hist_Err36, tracks2hist_Err44, tracks2hist_Err45,
                       tracks2hist_Err46, tracks2hist_Err55, tracks2hist_Err56, tracks2hist_Err66])

    tracks2hist.param('fileName', outputRootFile)
    path.add_module(tracks2hist)


def addNeutralsConversionMonitors(gammaOutputRootFile='b2biiGammaConversionMonitors.root',
                                  neutralPiOutputRootFile='b2biiPi0ConversionMonitors.root',
                                  MCneutralPiOutputRootFile='b2biiMCPi0ConversionMonitors.root',
                                  path=None):
    """
    Creates 'gamma:b2bii_monitor' and 'pi0:b2bii_monitor' from already existing 'gamma:mdst' and
    'pi0:mdst' ParticleList and fills it with all converted neutral gammas.
    For each object several quantities are stored to histograms with 'VariablesToHistogram' module
    for monitoring purpuses: e.g. to be compared with same distributions obtained within BASF.

    @param outputRootFile name of the output ROOT file to which the histograms are saved.
    @param path modules are added to this path
    """

    # load gammas and pi0, copy pi0s from 'pi0:mdst' list. We don't want to mess with them.
    copyList('gamma:b2bii_monitor', 'gamma:mdst', False, path)
    copyParticles('pi0:b2bii_monitor', 'pi0:mdst', False, path)
    kFit('pi0:b2bii_monitor', -1, 'mass', path=path)
    matchMCTruth('gamma:b2bii_monitor', path)
    matchMCTruth('pi0:b2bii_monitor', path)

    cutAndCopyLists('pi0:b2bii_monitorMC', 'pi0:b2bii_monitor', 'mcPDG == 111', path=path)

    # register VariablesToHistogram and fill them with monitored variables
    gamma2hist = b2.register_module('VariablesToHistogram')
    gamma2hist.param('particleList', 'gamma:b2bii_monitor')
    neutralPi2hist = b2.register_module('VariablesToHistogram')
    neutralPi2hist.param('particleList', 'pi0:b2bii_monitor')

    MCneutralPi2hist = b2.register_module('VariablesToHistogram')
    MCneutralPi2hist.param('particleList', 'pi0:b2bii_monitorMC')

    # define variables that are monitored and specify
    # the corresponding histogram (#bins, low, high)
    # ('variable_name', number_of_bins, x_low, x_high)

    ###################
    # Gamma
    ###################

    # Position and momentum
    gamma2hist_x = ('x', 100, -1.0, 1.0)
    gamma2hist_y = ('y', 100, -1.0, 1.0)
    gamma2hist_z = ('z', 100, -1.0, 1.0)
    gamma2hist_px = ('px', 100, -1.5, 1.5)
    gamma2hist_py = ('py', 100, -1.5, 1.5)
    gamma2hist_pz = ('pz', 100, -1.0, 2.5)
    gamma2hist_truepx = ('mcPX', 100, -1.5, 1.5)
    gamma2hist_truepy = ('mcPY', 100, -1.5, 1.5)
    gamma2hist_truepz = ('mcPZ', 100, -1.0, 2.5)
    gamma2hist_trueE = ('mcE', 100, 0.0, 3.0)

    # Cluster variables
    gamma2hist_E = ('E', 100, 0.0, 3.0)
    gamma2hist_Theta = ('clusterTheta', 100, 0.0, 3.14)
    gamma2hist_Phi = ('clusterPhi', 100, -3.14, 3.14)
    gamma2hist_R = ('clusterR', 100, 120, 260)

    # Auxiliary variables
    gamma2hist_Edep = ('clusterUncorrE', 100, 0.0, 0.1)
    # gamma2hist_Time= ('clusterTiming', 100, ?, ?)
    gamma2hist_Emax = ('clusterHighestE', 100, 0.0, 2.0)
    gamma2hist_E9E25 = ('clusterE9E25', 100, 0.0, 1.0)
    gamma2hist_noC = ('clusterNHits', 100, 0.0, 30.0)
    gamma2hist_Quality = ('clusterBelleQuality', 10, 0.0, 20.0)
    gamma2hist_Width = ('clusterLAT', 100, 0.0, 50.0)

    gamma2hist_Err00 = ('momVertCovM(0,0)', 100, -0.0005, 0.0005)
    gamma2hist_Err10 = ('momVertCovM(1,0)', 100, -0.0005, 0.0005)
    gamma2hist_Err11 = ('momVertCovM(1,1)', 100, -0.0005, 0.0005)
    gamma2hist_Err20 = ('momVertCovM(2,0)', 100, -0.0005, 0.0005)
    gamma2hist_Err21 = ('momVertCovM(2,1)', 100, -0.0005, 0.0005)
    gamma2hist_Err22 = ('momVertCovM(2,2)', 100, -0.0005, 0.0005)
    gamma2hist_Err30 = ('momVertCovM(3,0)', 100, -0.0005, 0.0005)
    gamma2hist_Err31 = ('momVertCovM(3,1)', 100, -0.0005, 0.0005)
    gamma2hist_Err32 = ('momVertCovM(3,2)', 100, -0.0005, 0.0005)
    gamma2hist_Err33 = ('momVertCovM(3,3)', 100, -0.0005, 0.0005)
    gamma2hist_Err40 = ('momVertCovM(4,0)', 100, -0.0005, 0.0005)
    gamma2hist_Err41 = ('momVertCovM(4,1)', 100, -0.0005, 0.0005)
    gamma2hist_Err42 = ('momVertCovM(4,2)', 100, -0.0005, 0.0005)
    gamma2hist_Err43 = ('momVertCovM(4,3)', 100, -0.0005, 0.0005)
    gamma2hist_Err44 = ('momVertCovM(4,4)', 100, 0.99, 1.01)
    gamma2hist_Err50 = ('momVertCovM(5,0)', 100, -0.0005, 0.0005)
    gamma2hist_Err51 = ('momVertCovM(5,1)', 100, -0.0005, 0.0005)
    gamma2hist_Err52 = ('momVertCovM(5,2)', 100, -0.0005, 0.0005)
    gamma2hist_Err53 = ('momVertCovM(5,3)', 100, -0.0005, 0.0005)
    gamma2hist_Err54 = ('momVertCovM(5,4)', 100, -0.0005, 0.0005)
    gamma2hist_Err55 = ('momVertCovM(5,5)', 100, 0.99, 1.01)
    gamma2hist_Err60 = ('momVertCovM(6,0)', 100, -0.0005, 0.0005)
    gamma2hist_Err61 = ('momVertCovM(6,1)', 100, -0.0005, 0.0005)
    gamma2hist_Err62 = ('momVertCovM(6,2)', 100, -0.0005, 0.0005)
    gamma2hist_Err63 = ('momVertCovM(6,3)', 100, -0.0005, 0.0005)
    gamma2hist_Err64 = ('momVertCovM(6,4)', 100, -0.0005, 0.0005)
    gamma2hist_Err65 = ('momVertCovM(6,5)', 100, -0.0005, 0.0005)
    gamma2hist_Err66 = ('momVertCovM(6,6)', 100, 0.99, 1.01)

    gamma2hist.param('variables',
                     [gamma2hist_x,
                      gamma2hist_y,
                      gamma2hist_z,
                      gamma2hist_px,
                      gamma2hist_py,
                      gamma2hist_pz,
                      gamma2hist_truepx,
                      gamma2hist_truepy,
                      gamma2hist_truepz,
                      gamma2hist_trueE,
                      gamma2hist_E,
                      gamma2hist_Theta,
                      gamma2hist_Phi,
                      gamma2hist_R,
                      gamma2hist_Edep,
                      gamma2hist_Emax,
                      gamma2hist_E9E25,
                      gamma2hist_noC,
                      gamma2hist_Quality,
                      gamma2hist_Width,
                      gamma2hist_Err00,
                      gamma2hist_Err10,
                      gamma2hist_Err11,
                      gamma2hist_Err20,
                      gamma2hist_Err21,
                      gamma2hist_Err22,
                      gamma2hist_Err30,
                      gamma2hist_Err31,
                      gamma2hist_Err32,
                      gamma2hist_Err33,
                      gamma2hist_Err40,
                      gamma2hist_Err41,
                      gamma2hist_Err42,
                      gamma2hist_Err43,
                      gamma2hist_Err44,
                      gamma2hist_Err50,
                      gamma2hist_Err51,
                      gamma2hist_Err52,
                      gamma2hist_Err53,
                      gamma2hist_Err54,
                      gamma2hist_Err55,
                      gamma2hist_Err60,
                      gamma2hist_Err61,
                      gamma2hist_Err62,
                      gamma2hist_Err63,
                      gamma2hist_Err64,
                      gamma2hist_Err65,
                      gamma2hist_Err66])

    ###################
    # Pi0
    ###################

    # Position and momentum
    neutralPi2hist_x = ('x', 100, -1.0, 1.0)
    neutralPi2hist_y = ('y', 100, -1.0, 1.0)
    neutralPi2hist_z = ('z', 100, -1.0, 1.0)
    neutralPi2hist_px = ('px', 100, -2.0, 2.0)
    neutralPi2hist_py = ('py', 100, -2.0, 2.0)
    neutralPi2hist_pz = ('pz', 100, -1.0, 2.5)
    neutralPi2hist_E = ('E', 100, 0.0, 3.0)
    neutralPi2hist_InvM = ('M', 100, 0.1344, 0.136)
    neutralPi2hist_M = ('InvM', 50, 0.08, 0.18)

    neutralPi2hist_truepx = ('mcPX', 100, -2.0, 2.0)
    neutralPi2hist_truepy = ('mcPY', 100, -2.0, 2.0)
    neutralPi2hist_truepz = ('mcPZ', 100, -1.0, 2.5)
    neutralPi2hist_trueE = ('mcE', 100, 0.0, 3.0)

    neutralPi2hist_d1x = ('daughter(0,x)', 100, -1.0, 1.0)
    neutralPi2hist_d1y = ('daughter(0,y)', 100, -1.0, 1.0)
    neutralPi2hist_d1z = ('daughter(0,z)', 100, -1.0, 1.0)
    neutralPi2hist_d1px = ('daughter(0,px)', 100, -1.5, 1.5)
    neutralPi2hist_d1py = ('daughter(0,py)', 100, -1.5, 1.5)
    neutralPi2hist_d1pz = ('daughter(0,pz)', 100, -1.0, 2.5)
    neutralPi2hist_d1e = ('daughter(0,E)', 100, 0.0, 3)

    neutralPi2hist_d2x = ('daughter(1,x)', 100, -1.0, 1.0)
    neutralPi2hist_d2y = ('daughter(1,y)', 100, -1.0, 1.0)
    neutralPi2hist_d2z = ('daughter(1,z)', 100, -1.0, 1.0)
    neutralPi2hist_d2px = ('daughter(1,px)', 100, -1.5, 1.5)
    neutralPi2hist_d2py = ('daughter(1,py)', 100, -1.5, 1.5)
    neutralPi2hist_d2pz = ('daughter(1,pz)', 100, -1.0, 2.5)
    neutralPi2hist_d2e = ('daughter(1,E)', 100, 0.0, 3)

    neutralPi2hist_Err00 = ('momVertCovM(0,0)', 100, -0.0005, 0.0005)
    neutralPi2hist_Err10 = ('momVertCovM(1,0)', 100, -0.0005, 0.0005)
    neutralPi2hist_Err11 = ('momVertCovM(1,1)', 100, -0.0005, 0.0005)
    neutralPi2hist_Err20 = ('momVertCovM(2,0)', 100, -0.0005, 0.0005)
    neutralPi2hist_Err21 = ('momVertCovM(2,1)', 100, -0.0005, 0.0005)
    neutralPi2hist_Err22 = ('momVertCovM(2,2)', 100, -0.0005, 0.0005)
    neutralPi2hist_Err30 = ('momVertCovM(3,0)', 100, -0.0005, 0.0005)
    neutralPi2hist_Err31 = ('momVertCovM(3,1)', 100, -0.0005, 0.0005)
    neutralPi2hist_Err32 = ('momVertCovM(3,2)', 100, -0.0005, 0.0005)
    neutralPi2hist_Err33 = ('momVertCovM(3,3)', 100, -0.0005, 0.0005)
    neutralPi2hist_Err40 = ('momVertCovM(4,0)', 100, -0.0005, 0.0005)
    neutralPi2hist_Err41 = ('momVertCovM(4,1)', 100, -0.0005, 0.0005)
    neutralPi2hist_Err42 = ('momVertCovM(4,2)', 100, -0.0005, 0.0005)
    neutralPi2hist_Err43 = ('momVertCovM(4,3)', 100, -0.0005, 0.0005)
    neutralPi2hist_Err44 = ('momVertCovM(4,4)', 100, -100.0, 100.0)
    neutralPi2hist_Err50 = ('momVertCovM(5,0)', 100, -0.0005, 0.0005)
    neutralPi2hist_Err51 = ('momVertCovM(5,1)', 100, -0.0005, 0.0005)
    neutralPi2hist_Err52 = ('momVertCovM(5,2)', 100, -0.0005, 0.0005)
    neutralPi2hist_Err53 = ('momVertCovM(5,3)', 100, -0.0005, 0.0005)
    neutralPi2hist_Err54 = ('momVertCovM(5,4)', 100, -100.0, 100.0)
    neutralPi2hist_Err55 = ('momVertCovM(5,5)', 100, -100.0, 100.0)
    neutralPi2hist_Err60 = ('momVertCovM(6,0)', 100, -0.0005, 0.0005)
    neutralPi2hist_Err61 = ('momVertCovM(6,1)', 100, -0.0005, 0.0005)
    neutralPi2hist_Err62 = ('momVertCovM(6,2)', 100, -0.0005, 0.0005)
    neutralPi2hist_Err63 = ('momVertCovM(6,3)', 100, -0.0005, 0.0005)
    neutralPi2hist_Err64 = ('momVertCovM(6,4)', 100, -100.0, 100.0)
    neutralPi2hist_Err65 = ('momVertCovM(6,5)', 100, -100.0, 100.0)
    neutralPi2hist_Err66 = ('momVertCovM(6,6)', 100, -100.0, 100.0)

    neutralPi2hist.param('variables',
                         [neutralPi2hist_x,
                          neutralPi2hist_y,
                          neutralPi2hist_z,
                          neutralPi2hist_px,
                          neutralPi2hist_py,
                          neutralPi2hist_pz,
                          neutralPi2hist_E,
                          neutralPi2hist_M,
                          neutralPi2hist_InvM,
                          neutralPi2hist_d1x,
                          neutralPi2hist_d1y,
                          neutralPi2hist_d1z,
                          neutralPi2hist_d1px,
                          neutralPi2hist_d1py,
                          neutralPi2hist_d1pz,
                          neutralPi2hist_d1e,
                          neutralPi2hist_d2x,
                          neutralPi2hist_d2y,
                          neutralPi2hist_d2z,
                          neutralPi2hist_d2px,
                          neutralPi2hist_d2py,
                          neutralPi2hist_d2pz,
                          neutralPi2hist_d2e,
                          neutralPi2hist_Err00,
                          neutralPi2hist_Err10,
                          neutralPi2hist_Err11,
                          neutralPi2hist_Err20,
                          neutralPi2hist_Err21,
                          neutralPi2hist_Err22,
                          neutralPi2hist_Err30,
                          neutralPi2hist_Err31,
                          neutralPi2hist_Err32,
                          neutralPi2hist_Err33,
                          neutralPi2hist_Err40,
                          neutralPi2hist_Err41,
                          neutralPi2hist_Err42,
                          neutralPi2hist_Err43,
                          neutralPi2hist_Err44,
                          neutralPi2hist_Err50,
                          neutralPi2hist_Err51,
                          neutralPi2hist_Err52,
                          neutralPi2hist_Err53,
                          neutralPi2hist_Err54,
                          neutralPi2hist_Err55,
                          neutralPi2hist_Err60,
                          neutralPi2hist_Err61,
                          neutralPi2hist_Err62,
                          neutralPi2hist_Err63,
                          neutralPi2hist_Err64,
                          neutralPi2hist_Err65,
                          neutralPi2hist_Err66])

    MCneutralPi2hist.param('variables', [neutralPi2hist_truepx, neutralPi2hist_truepy, neutralPi2hist_truepz, neutralPi2hist_trueE])

    gamma2hist.param('fileName', gammaOutputRootFile)
    neutralPi2hist.param('fileName', neutralPiOutputRootFile)
    MCneutralPi2hist.param('fileName', MCneutralPiOutputRootFile)

    path.add_module(gamma2hist)
    path.add_module(neutralPi2hist)
    path.add_module(MCneutralPi2hist)


def addMCParticlesConversionMonitors(outputRootFile='b2biiMCParticlesConversionMonitors.root', path=None):
    """
    'HistoManager' and 'B2BIIMCParticlesMonitor' modules create and fill the monitoring
    histograms using all MCParticles.
    @param outputRootFile name of the output ROOT file to which the histograms are saved.
    @param path modules are added to this path
    """

    histo = b2.register_module("HistoManager")
    MCParticles2hist = b2.register_module('B2BIIMCParticlesMonitor')

    histo.param("histoFileName", outputRootFile)

    path.add_module(histo)
    path.add_module(MCParticles2hist)
