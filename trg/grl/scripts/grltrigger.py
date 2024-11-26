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

    grl = b2.register_module('GRLNeuro')
    grl.param('nMLP', 1)
    grl.param('multiplyHidden', False)
    grl.param('nHidden', [[20, 20]])
    grl.param('n_cdc_sector', 0)
    grl.param('n_ecl_sector', 1)
    grl.param('i_cdc_sector', [0*35*3])
    grl.param('i_ecl_sector', [1*6*3+1])
    grl.param('weightFiles', [b2.find_file("data/trg/grl/weights.dat")])
    grl.param('biasFiles', [b2.find_file("data/trg/grl/bias.dat")])
    path.add_module(grl)

    objects = b2.register_module('TRGGRLProjects')
    objects.param('SimulationMode', SimulationMode)
    path.add_module(objects)
