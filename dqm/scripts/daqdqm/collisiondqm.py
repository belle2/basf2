#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


from daqdqm.commondqm import add_common_dqm
from IPDQM import add_IP_dqm
from V0DQM import add_V0_dqm


def add_collision_dqm(path, components=None, dqm_environment="expressreco", dqm_mode="dont_care", create_hlt_unit_histograms=False):
    """
    This function adds DQMs for collisions

    @param components: A list of the detector components which are available in this
                       run of basf2
    @param dqm_environment: The environment the DQM modules are running in
                            "expressreco" (default) if running on the ExpressReco system
                            "hlt" if running on the HLT online reconstructon nodes
                            If running on the hlt, you may want to output less or other DQM plots
                            due to the limited bandwith of the HLT nodes.
    @param dqm_mode: How to split up the path for online/HLT.
                     For dqm_mode == "dont_care" all the DQM modules should be added.
                     For dqm_mode == "all_events" only the DQM modules which should run on all events
                            (filtered and dismissed) should be added
                     For dqm_mode == "before_filter" only the DQM modules which should run before
                            the hlt filter
                     For dqm_mode == "filtered"  only the DQM modules which should run on filtered
                            events should be added
    @param create_hlt_unit_histograms: Parameter for SoftwareTiggerHLTDQMModule.
                                         Should be True only when running on the HLT servers
    """

    assert dqm_mode in ["dont_care", "all_events", "filtered", "before_filter"]

    add_common_dqm(path, components=components, dqm_environment=dqm_environment,
                   dqm_mode=dqm_mode, create_hlt_unit_histograms=create_hlt_unit_histograms)

    if dqm_environment == "expressreco" and (dqm_mode in ["dont_care"]):

        # PXD (not useful on HLT)
        if components is None or 'PXD' in components:
            # need to be behind add_common_dqm as intercepts are calculated there
            path.add_module('PXDDQMEfficiency', histogramDirectoryName='PXDEFF')

    if dqm_mode in ["dont_care", "filtered"]:

        # IP DQM (when run on HLT, less plots are produced)
        if components is None or ('CDC' in components and 'SVD' in components):
            add_IP_dqm(path, dqm_environment=dqm_environment)

    # the following makes only sense in collisions
    if dqm_environment == "expressreco" and (dqm_mode in ["dont_care"]):

        # V0 DQM
        if components is None or 'CDC' in components:
            add_V0_dqm(path)
