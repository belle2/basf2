# !/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2


def add_grl_trigger(path, SimulationMode=1):
    """
    do match and produce the objects on grl
    @path modules are added to this path
    @SimulationMode 0: fast simulation 1:full simulation
    """
    match = b2.register_module('TRGGRLMatch')
    match.param('SimulationMode', SimulationMode)
    path.add_module(match)

    grlnn = b2.register_module('GRLNeuro')
    path.add_module(grlnn)

    _TRGGRLInfo = 'TRGGRLObjects'

    path.add_module(
        'TRGGRLProjects',
        SimulationMode=SimulationMode,
        TrgGrlInformation=_TRGGRLInfo
    )

    path.add_module(
        'TRGGRLInjectionVetoFromOverlay',
        TRGGRLInfoName=_TRGGRLInfo
    )
