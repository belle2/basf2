#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import fillParticleList
from modularAnalysis import analysis_main


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
    beam2hist_Eher = ('Eher', 100, 7.990, 8.00)
    beam2hist_Eler = ('Eler', 100, 3.498, 3.50)
    beam2hist_Ecms = ('Ecms', 100, 11.49, 11.50)
    beam2hist_Xang = ('XAngle', 100, -0.03, 0.03)

    beam2hist_ipx = ('IPX', 100, -0.1, 0.1)
    beam2hist_ipy = ('IPY', 100, -0.1, 0.1)
    beam2hist_ipz = ('IPZ', 100, -1.0, 1.0)

    beam2hist.param('variables', [beam2hist_Eher, beam2hist_Eler, beam2hist_Ecms, beam2hist_Xang,
                                  beam2hist_ipx, beam2hist_ipy, beam2hist_ipz])

    beam2hist.param('fileName', outputRootFile)
    path.add_module(beam2hist)


def addTrackConversionMonitors(outputRootFile='b2biiTrackConversionMonitors.root', path=analysis_main):
    """
    Creates 'pi+:b2bii_monitor' ParticleLists and fills it with all converted charged tracks as charged pions.
    For each charged pion (track) several quantities are stored to histograms with 'VariablesToHistogram' module
    for monitoring purpuses: e.g. to be compared with same distributions obtained within BASF.

    @param outputRootFile name of the output ROOT file to which the histograms are saved.
    @param path modules are added to this path
    """

    # create charged pions from all charged tracks
    fillParticleList('pi+:b2bii_monitor', '')

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
