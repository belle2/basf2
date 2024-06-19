#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


from daqdqm.commondqm import add_common_dqm


def add_cosmic_dqm(path, components=None, dqm_environment="expressreco", dqm_mode="dont_care", create_hlt_unit_histograms=False):
    """
    This function adds DQMs for CRT

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
                            all reconstruction
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
            # disable d0 and z0 cut for cosmics
            path.add_module('PXDDQMEfficiency', histogramDirectoryName='PXDEFF', z0minCut=-9999, z0maxCut=9999, d0Cut=9999)
