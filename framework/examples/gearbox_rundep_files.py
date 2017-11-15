#!/usr/bin/env python3
# -*- coding: utf-8 -*-

############################################################
# This steering file shows how to use rundependent xml files
#
# Example steering file - 2014 Belle II Collaboration
############################################################

from basf2 import *

main = create_path()
# EventInfoSetter - generate event meta data
main.add_module('EventInfoSetter', expList=[71, 71, 73], runList=[3, 4, 10], evtNumList=[1, 1, 1])

# LogLevel.INFO will print all filenames if opend by a run-dependent backend
gearbox = main.add_module("Gearbox", logLevel=LogLevel.INFO)
gearbox.param({
    # For each xml file "foo/bar.xml" requested the gearbox will look for
    "backends": [
        # rundata-071-0003/foo/bar.xml{.gz}
        "file:rundata-{EXP:3}-{RUN:4}/",
        # {EXP} and {RUN} will be replaced by actual exp/run number
        # {EXP:n} and {RUN:n} will be replaced with exp/run number with n
        # leading zeros
        # To allow flat structure it will look for
        #   rundata-071-0003/foo/bar.xml{.gz}
        #   rundata-071-0003/foo-bar.xml{.gz}
        #   rundata-071-0003-foo-bar.xml{.gz}

        # $BELLE2_RUN_DIR/data/foo/bar.xml{.gz}
        "file:"
    ],
})
# If it cannot find the file in the first backend it will fall back to the
# second and so on, e.g. one could have a
# rundata-071-0003-pxd-PXD-Alignment.xml
# to override the pxd/PXD-Alignment.xml

# Process all events
process(main)
print(statistics(statistics.BEGIN_RUN))
