#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2


def add_ecl_trigger(path, SourceOfTC=3, ETMConditionDB=1):
    """
    add ecl trigger module to path
    """
    trgeclfam = b2.register_module("TRGECLFAM")
    # Output TC Waveform (0 : no save, 1 : save)
    trgeclfam.param('TCWaveform', 0)
    # save only measured TC data(=0) or both measured and true TC data(=1)
    trgeclfam.param('FAMAnaTable', 0)
    # select source of TC data (1=ECLHit, 2=ECLSimHit, 3=ECLHit+TRGECLBGTCHit)
    # ("1:=ECLHit" is used for signal w/o bkg, and real time background monitor)
    trgeclfam.param('SourceOfTC', SourceOfTC)
    #
    path.add_module(trgeclfam)
    #
    trgecl = b2.register_module("TRGECL")
    # trgecl.logging.log_level = LogLevel.DEBUG

    # flag of ConditionDB (0:=not use ConditionDB, 1:=use ConditionDB)
    trgecl.param('ConditionDB', ETMConditionDB)

    # Output Clustering method(0: Use only ICN, 1: ICN + Max TC)
    trgecl.param('Clustering', 1)
    # The limit # of cluster in clustering logic
    trgecl.param('ClusterLimit', 6)

    # Theta ID region(low and high) of 3DBhabhaVetoInTrack
    trgecl.param('3DBhabhaVetoInTrackThetaRegion', [3, 15])
    # taub2b two cluster angle selection in CM (degree)
    # (phi low, phi high, theta low, theta high)
    trgecl.param('Taub2bAngleCut', [110, 250, 130, 230])
    # taub2b total energy cut in lab (GeV)
    trgecl.param('Taub2bEtotCut', 7.0)
    # taub2b cluster energy selection in lab (GeV)
    trgecl.param('Taub2bCLELabCut', 1.9)

    # taub2b2 two cluster angle selection in CM (degree)
    # (phi low, phi high, theta low, theta high)
    trgecl.param('Taub2b2AngleCut', [120, 240, 140, 220])
    # taub2b2 total energy cut in lab (GeV)
    trgecl.param('Taub2b2EtotCut', 7.0)
    # taub2b2 cluster energy cut(low, high) in lab (GeV)
    trgecl.param('Taub2b2CLELabCut', [0.162, 3.0])

    # taub2b3 two cluster angle selection in CM (degree)
    # (phi low, phi high, theta low, theta high)
    trgecl.param('Taub2b3AngleCut', [120, 240, 140, 220])
    # taub2b3 total energy cut in lab (GeV)
    trgecl.param('Taub2b3EtotCut', 7.0)
    # taub2b3 cluster energy cut in lab (GeV) for one of b2b cluseter
    trgecl.param('Taub2b3CLEb2bLabCut', 0.14)
    # taub2b3 cluster energy low selection(low, high) in lab (GeV) for all clusters
    trgecl.param('Taub2b3CLELabCut', [0.12, 4.5])

    # hie1 and hie2, angle selection for addition Bhabha veto
    trgecl.param('hie12BhabhaVetoAngle', [150, 210, 160, 200])
    #
    path.add_module(trgecl)
