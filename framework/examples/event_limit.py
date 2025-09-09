#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
This steering file demonstrates the capabilities of EventLimiter module.

EventInfoSetter generates 200 events.

Then, the filtering with EventLimiter makes it so that only 150 events
pass into EventInfoPrinter

If EventLimit payload was previously generated using add_event_limit.py,
then the event limit will be adjusted based on payload contents.
"""

import basf2 as b2

# Register necessary modules
# Create paths
main = b2.Path()

# Generate 200 events
main.add_module('EventInfoSetter', expList=[0], runList=[0], evtNumList=[200])

# If no EventLimit payload in the database, discard
# all events with event number >= 150.
#
# If EventLimit payload is available, get an actual
# threshold value instead of 150.
main.add_module('EventLimiter')
limiter = main.add_module('EventLimiter', loadFromDB=True, maxEventsPerRun=150)
limiter.if_value("==0", b2.Path(), b2.AfterConditionPath.END)

# Print event meta info
main.add_module('EventInfoPrinter')

# Load payloads from the local database
b2.conditions.reset()
b2.conditions.prepend_testing_payloads('localdb/database.txt')

# Process data
b2.process(main, calculateStatistics=True)
print(b2.statistics)
