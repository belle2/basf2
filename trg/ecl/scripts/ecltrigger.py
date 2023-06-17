#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2


def add_ecl_trigger(path):
    """
    add ecl trigger module to path
    """
    trgeclfam = b2.register_module("TRGECLFAM")
    # Output TC Waveform (0 : no save, 1 : save)
    trgeclfam.param('TCWaveform', 0)
    # save only measured TC data(=0) or both measured and true TC data(=1)
    trgeclfam.param('FAMAnaTable', 0)
    #
    path.add_module(trgeclfam)
    #
    trgecl = b2.register_module("TRGECL")
    # trgecl.logging.log_level = LogLevel.DEBUG

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
    # taub2b cluster energy selection in lab (GeV) : E(CL1) and E(CL2)
    trgecl.param('Taub2bClusterECut1', 1.9)
    trgecl.param('Taub2bClusterECut2', 999.0)

    # taub2b2 two cluster angle selection in CM (degree)
    # (phi low, phi high, theta low, theta high)
    trgecl.param('Taub2b2AngleCut', [120, 240, 140, 220])
    # taub2b2 total energy cut in lab (GeV)
    trgecl.param('Taub2b2EtotCut', 7.0)
    # taub2b2 cluster energy cut in lab (GeV) for cluseter in endcap
    trgecl.param('Taub2b2CLEEndcapCut', 3.0)
    # taub2b2 cluster energy selection in lab (GeV)
    trgecl.param('Taub2b2CLECut', 0.162)

    # taub2b3 two cluster angle selection in CM (degree)
    # (phi low, phi high, theta low, theta high)
    trgecl.param('Taub2b3AngleCut', [120, 240, 140, 220])
    # taub2b3 total energy cut in lab (GeV)
    trgecl.param('Taub2b3EtotCut', 7.0)
    # taub2b3 cluster energy cut in lab (GeV) for one of b2b cluseter
    trgecl.param('Taub2b3CLEb2bCut', 0.14)
    # taub2b3 cluster energy low selection in lab (GeV) for all clusters
    trgecl.param('Taub2b3CLELowCut', 0.12)
    # taub2b3 cluster energy high selection in lab (GeV) for all clusters
    trgecl.param('Taub2b3CLEHighCut', 4.5)

    # angle selection for addition Bhabha veto
    trgecl.param('3DBhabhaAddAngleCut', [150, 210, 160, 200])
    #
    path.add_module(trgecl)
