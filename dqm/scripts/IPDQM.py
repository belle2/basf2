# !/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
This module defines functions to add the interaction point montitor to DQM modules.
"""

import basf2
import modularAnalysis as ma
import vertex as vx


def add_IP_dqm(path, dqm_environment='expressreco'):
    '''
    This function adds the IPDQM module to the path.
    @param dqm_environment: The environment the DQM modules are running in
                            "expressreco" (default) if running on the ExpressReco system
                            "hlt" if running on the HLT online reconstructon nodes
    '''

    mySelection = '[p > 1.0] and [abs(dz) < 2.0] and [dr < 0.5]'
    if dqm_environment == 'expressreco':
        ma.fillParticleList('mu+:DQM', mySelection, path=path)
        ma.reconstructDecay('Upsilon(4S):IPDQM -> mu+:DQM mu-:DQM', '9.5<M<11.5', path=path)
        vx.kFit('Upsilon(4S):IPDQM', conf_level=0, path=path)

        dqm = basf2.register_module('IPDQM')
        dqm.set_log_level(basf2.LogLevel.INFO)
        dqm.param('Y4SPListName', 'Upsilon(4S):IPDQM')
        dqm.param('onlineMode', dqm_environment)
        path.add_module(dqm)

    elif dqm_environment == 'hlt':
        # In order to save computing time, we fill the ParticleList only if the event passes the dimuon skim.
        mySelection += ' and [SoftwareTriggerResult(software_trigger_cut&skim&accept_mumu_tight_or_highm) > 0]'
        ma.fillParticleList('mu+:DQM_HLT', mySelection, path=path)
        ma.reconstructDecay('Upsilon(4S):IPDQM_HLT -> mu+:DQM_HLT mu-:DQM_HLT', '9.5<M<11.5', path=path)
        vx.kFit('Upsilon(4S):IPDQM_HLT', conf_level=0, path=path)

        dqm = basf2.register_module('IPDQM')
        dqm.set_log_level(basf2.LogLevel.INFO)
        dqm.param('Y4SPListName', 'Upsilon(4S):IPDQM_HLT')
        dqm.param('onlineMode', dqm_environment)
        path.add_module(dqm)
