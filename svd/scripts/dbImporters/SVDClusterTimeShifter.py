#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
#                                                                        #
# Usage: basf2 <script> -- -j ../../data/SVDClusterTimeShift.json        #
#                                                                        #
# Add shift values of other time algorithm in json file if needed.       #
#                                                                        #
##########################################################################


import basf2 as b2
from ROOT import Belle2
import datetime
import json
import argparse

now = datetime.datetime.now()


def arg_parser():
    """ argument parser """

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-j', '--json',
                        type=str,
                        help='input json file',
                        metavar='JSON')
    return parser


class timeClusterTimeShifterImporter(b2.Module):
    ''' importer of the SVDClusterTimeShifter'''

    def beginRun(self):
        '''begin run'''

        print("")
        print("--> json INFO:")
        print("              "+str(param["_COMMENT"]))
        print("")

        iov = Belle2.IntervalOfValidity.always()

        uniqueID = "SVDClusterTimeShifter_" + str(now.isoformat())
        uniqueID += "_in-" + param["uniqueID"]["INFO"]["source"]
        uniqueID += "_useFor-" + param["uniqueID"]["INFO"]["useFor"]
        uniqueID += "_" + param["uniqueID"]["INFO"]["special"]
        uniqueID += "_" + param["uniqueID"]["INFO"]["tag"]
        print("uniqueID ->", uniqueID)
        print("")

        payload = Belle2.SVDClusterTimeShifter(uniqueID, param["_DESCRIPTION"])

        for alg in param["shiftValues"]:
            for sType in param["shiftValues"][alg]:
                payload.setClusterTimeShift(alg, sType, param["shiftValues"][alg][sType])

        # write out the payload to localdb directory
        Belle2.Database.Instance().storeData(Belle2.SVDClusterTimeShifter.name, payload, iov)


main = b2.create_path()

# Argument parsing
args = arg_parser().parse_args()

# open json file
f = open(args.json)
param = json.load(f)


# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module(timeClusterTimeShifterImporter())

main.add_module('Progress')

b2.print_path(main)
b2.process(main)
print(b2.statistics)
