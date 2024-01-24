#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""Custom calibration strategy for KLM alignment."""

from caf.strategies import SequentialRunByRun
from caf.utils import IoV


class KLMAlignment(SequentialRunByRun):
    """
    Custom stragery for KLM alignment. Based on SequentialRunByRun strategy,
    but requires custom experiment-dependent settings.
    """
    #: The params that you could set on the Algorithm object which this Strategy would use.
    #: Just here for documentation reasons.
    usable_params = {
        "has_experiment_settings": bool,  # Used in SequentialRunByRun.
        "iov_coverage": IoV,              # Used in SequentialRunByRun.
        "millepede_entries": int,
        "millepede_entries_exp7": int,
        "step_size": int                  # Used in SequentialRunByRun.
    }

    #: Granularity of collector that can be run by this algorithm properly
    allowed_granularities = ["run"]

    def __init__(self, algorithm):
        """
        """
        super().__init__(algorithm)
        #: Number of Millepede entries.
        self.millepede_entries = 0
        #: Number of Millepede entries for experiment 7.
        self.millepede_entries_exp7 = 0

    def apply_experiment_settings(self, algorithm, experiment):
        """
        Apply experiment-dependent settings.
        """
        if "millepede_entries" in self.algorithm.params:
            self.millepede_entries = self.algorithm.params["millepede_entries"]

        if "millepede_entries_exp7" in self.algorithm.params:
            self.millepede_entries_exp7 = self.algorithm.params["millepede_entries_exp7"]
        if experiment == 7:
            algorithm.setMinEntries(self.millepede_entries_exp7)
        else:
            algorithm.setMinEntries(self.millepede_entries)
