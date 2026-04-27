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

    # Execute SoftwareTrigger module for prefilter
    path.add_module("SoftwareTrigger", baseIdentifier="prefilter")

    # DQM for prefilter to save decision of individual prefilter lines
    cutResultIdentifiers = {}
    hlt_prefilter_lines_in_plot = []
    from softwaretrigger import prefilter_categories
    prefilter_cat = [method for method in dir(prefilter_categories) if method.startswith('__') is False if method != 'RESULTS']

    def read_lines(category):
        return [i.split(" ", 1)[1].replace(" ", "_") for i in category]
    for i in prefilter_cat:
        hlt_prefilter_lines_in_plot += read_lines(getattr(prefilter_categories, i))

    cutResultIdentifiers["prefilter"] = {"prefilter": hlt_prefilter_lines_in_plot}

    path.add_module("SoftwareTriggerHLTDQM",
                    cutResultIdentifiers=cutResultIdentifiers,
                    createHLTUnitHistograms=False,
                    createTotalResultHistograms=False,
                    createErrorFlagHistograms=False,
                    createExpRunEventHistograms=False,
                    histogramDirectoryName="softwaretrigger_before_prefilter",
                    pathLocation="before prefilter")

    # Get total_result for prefilter
    hlt_prefilter_module = path.add_module("TriggerSkim", triggerLines=["software_trigger_cut&prefilter&total_result"])

    # Filter events rejected by prefilter, only save event metadata
    hlt_event_abort(hlt_prefilter_module, "<1", ROOT.Belle2.EventMetaData.c_HLTPrefilterDiscard)

    # Save module statistics
    path.add_module('StatisticsSummary').set_name('Sum_HLTPrefilter')
