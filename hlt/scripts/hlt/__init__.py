##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
from softwaretrigger.path_utils import hlt_event_abort


def add_prefilter_module(path):
    """
    Add the SoftwareTrigger for the HLT prefilter cuts to the given path.
    Only the calculation of the cuts is implemented here - the cut logic has to be done
    using the module return value.
    Save the result of HLTPrefilter lines to HLTDQM.
    Discard events tagged by HLTPrefilter as injection background or high occupancy.
    """

    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa

    path.add_module("SoftwareTrigger", baseIdentifier="prefilter")
    hlt_prefilter_module = path.add_module("TriggerSkim", triggerLines=["software_trigger_cut&prefilter&total_result"])
    hlt_event_abort(hlt_prefilter_module, "<1", ROOT.Belle2.EventMetaData.c_HLTPrefilterDiscard)
    path.add_module('StatisticsSummary').set_name('Sum_HLTPrefilter')
