#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Purpose:
#   basf module to return false if the event counter equals or exceeds the user-defined limit
#
import basf2

# =========================================================================
#
#   EventCountLimiter
#
# =========================================================================


class EventCountLimiter(basf2.Module):
    """Report when the event counter has reached or exceeded its limit"""

    def __init__(self, maxCount):
        """Constructor

        Argument:
            maxCount (int): number of events to be processed
        """
        super().__init__()
        #: internal copy of the event-counter upper limit
        self.maxCount = maxCount if (maxCount >= 0) else (1 << 31)
        #: internal event counter
        self.eventCounter = 0

    def initialize(self):
        """Handle job initialization (nothing to do here)"""

    def terminate(self):
        """Handle job termination (nothing to do here)"""

    def beginRun(self):
        """Handle begin of run (nothing to do here)"""

    def endRun(self):
        """Handle end of run (nothing to do here)"""

    def event(self):
        """Process one event: has eventCounter reached or exceeded its limit?"""
        self.eventCounter += 1
        super().return_value(self.eventCounter < self.maxCount)
