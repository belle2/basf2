#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Check for possible problems on the processing path that will affect HLT procesing

1. Conditions not continuing to main path

   On HLT we never want to just end processing, we always want to reach the end
   to send the event on to the output. So make sure no conditions with
   AfterConditionPath.END is present.

2. Multiple EventErrorFlags on one path

   We don't want to sent multiple error flags unconditionally after each other so
   if there are two ore more EventErrorFlags on one path this is wrong.
"""

import sys
import itertools
import basf2
from softwaretrigger import constants
from softwaretrigger.processing import add_hlt_processing


def short_path(path):
    """Just print a short version of the path"""
    return '[' + " -> ".join(m.name() for m in path.modules()) + ']'


def check_path(path):
    """Check the given path recursively"""
    result = True
    max_number = {
        'EventErrorFlag': 1
    }
    for m in path.modules():
        if m.type() in max_number:
            max_number[m.type()] -= 1
            if max_number[m.type()] < 0:
                basf2.B2ERROR("Too many identical modules in path", type=m.type(), path=short_path(path))
                result = False

        for c in m.get_all_conditions():
            if c.get_after_path() != basf2.AfterConditionPath.CONTINUE:
                basf2.B2ERROR(f"Condition on '{m.name()}' doesn't return to original path, potential loss of events",
                              condition=str(c))
                result = False
            result &= check_path(c.get_path())
    return result


if __name__ == "__main__":
    all_good = True
    for run_type, mode in itertools.product(constants.RunTypes, constants.SoftwareTriggerModes):
        basf2.B2INFO(f"Checking processing path for {run_type} in {mode} mode")
        path = basf2.Path()
        add_hlt_processing(path, run_type=run_type, softwaretrigger_mode=mode)
        basf2.print_path(path)
        if not check_path(path):
            basf2.B2ERROR(f"Problems found for {run_type} in {mode} mode")
            all_good = False

    sys.exit(0 if all_good else 1)
