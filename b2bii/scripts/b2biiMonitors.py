#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import fillParticleList
from modularAnalysis import copyParticles
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from vertex import vertexKFit


def addBeamParamsConversionMonitors(outputRootFile='b2biiBeamParamsConversionMonitors.root', path=analysis_main):
    """
    In each event saves HER, LER, CMS, x-angle to histograms for monitoring purpuses: e.g. to be compared with
    same distributions obtained within BASF.

    @param outputRootFile name of the output ROOT file to which the histograms are saved.
    @param path modules are added to this path
    """

    # register VariablesToHistogram and fill it with monitored variables
    beam2hist = register_module('VariablesToHistogram')
    beam2hist.param('particleList', '')

    # define variables that are monitored and specify
    # the corresponding histogram (#bins, low, high)
    # ('variable_name', number_of_bins, x_low, x_high)
    beam2hist_expno = ('expNum', 100, 0.0, 99.0)
    beam2hist_runno = ('modulo(runNum,1000)', 1000, 0.0, 999.0)
    beam2hist_evtno = ('modulo(evtNum,10000)', 10000, 0.0, 9999.0)

    beam2hist_Eher = ('Eher', 100, 7.990, 8.00)
    beam2hist_Eler = ('Eler', 100, 3.495, 3.50)
    beam2hist_Ecms = ('Ecms', 100, 10.50, 10.60)
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


def addKshortConversionMonitors(outputRootFile='b2biiKshortConversionMonitors.root', path=analysis_main):
    """
    Creates copies of KShort particles from 'K_S0:mdst' ParticleList and fills them to a new ParticleList called
    'K_S0:b2bii_monitor'. For each KShort candidate several quantities are stored to histograms with 'VariablesToHistogram' module
    for monitoring purpuses: e.g. to be compared with same distributions obtained within BASF.

    @param outputRootFile name of the output ROOT file to which the histograms are saved.
    @param path modules are added to this path
    """
    # copy KShorts from 'K_S0:mdst' list. We don't want to mess with them.
    copyParticles('K_S0:b2bii_monitor', 'K_S0:mdst', False, path)
    vertexKFit('K_S0:b2bii_monitor', -1, '', '', path)
    matchMCTruth('K_S0:b2bii_monitor', path)

    # register VariablesToHistogram and fill it with monitored variables
    kshorts2hist = register_module('VariablesToHistogram')
    kshorts2hist.param('particleList', 'K_S0:mdst')
    fittdKS2hist = register_module('VariablesToHistogram')
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
    fittdKS2hist_ksd0MCP = ('abs(daughter(0,mcP))', 100, 0.0, 3.5)
    fittdKS2hist_ksd1MCP = ('abs(daughter(1,mcP))', 100, 0.0, 3.5)

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


def addTrackConversionMonitors(outputRootFile='b2biiTrackConversionMonitors.root', path=analysis_main):
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
    tracks2hist = register_module('VariablesToHistogram')
    tracks2hist.param('particleList', 'pi+:b2bii_monitor')

    # define variables that are monitored and specify
    # the corresponding histogram (#bins, low, high)
    # ('variable_name', number_of_bins, x_low, x_high)
    tracks2hist_p = ('p', 100, 0.0, 3.0)
    tracks2hist_px = ('px', 100, -3.0, 3.0)

    tracks2hist_x = ('x', 100, -30.0, 30.0)

    tracks2hist_Err00 = ('momVertCovM(0,0)', 100, 0.0, 0.0001)

    tracks2hist_KID = ('atcPIDBelle(3,2)', 120, -0.1, 1.1)
    tracks2hist_PRKID = ('atcPIDBelle(4,3)', 120, -0.1, 1.1)
    tracks2hist_PRPID = ('atcPIDBelle(4,2)', 120, -0.1, 1.1)
    tracks2hist_MUID = ('muIDBelle', 120, -0.1, 1.1)
    tracks2hist_MUIDQ = ('muIDBelleQuality', 120, -0.1, 1.1)
    tracks2hist_EID = ('eIDBelle', 120, -0.1, 1.1)

    tracks2hist.param('variables', [tracks2hist_p, tracks2hist_px, tracks2hist_x,
                                    tracks2hist_Err00,
                                    tracks2hist_KID, tracks2hist_PRKID, tracks2hist_PRPID,
                                    tracks2hist_MUID, tracks2hist_MUIDQ, tracks2hist_EID])

    tracks2hist.param('fileName', outputRootFile)
    path.add_module(tracks2hist)
