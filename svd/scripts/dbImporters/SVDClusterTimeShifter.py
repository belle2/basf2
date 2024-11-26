#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
# Usage: basf2 <script> -- -j ../../data/SVDClusterTimeShift.json
# Add shift values of other time algorithm in json file if needed.
##########################################################################


from ROOT import Belle2
import basf2 as b2
import datetime
import json
import argparse
import math

now = datetime.datetime.now()


def arg_parser():
    """ argument parser """

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-j', '--json',
                        type=str,
                        help='input json file',
                        metavar='JSON')
    return parser


if __name__ == "__main__":

    # Argument parsing
    args = arg_parser().parse_args()

    # open json file
    f = open(args.json)
    param = json.load(f)

    print("")
    print("--> json INFO:")
    print("              "+str(param["_COMMENT"]))
    print("")

    iov = Belle2.IntervalOfValidity.always()

    uniqueID = "SVDClusterTimeShifter_" + str(now.isoformat())
    uniqueID += "_in-" + param["uniqueID"]["INFO"]["source"]
    uniqueID += "_useFor-" + param["uniqueID"]["INFO"]["useFor"]
    uniqueID += "_" + param["uniqueID"]["INFO"]["config"]
    uniqueID += "_" + param["uniqueID"]["INFO"]["tag"]
    print("uniqueID ->", uniqueID)
    print("")

    payload = Belle2.SVDClusterTimeShifter(uniqueID)

    for alg in param["shiftValues"]:
        for sType in param["shiftValues"][alg]:
            shiftVals = param["shiftValues"][alg][sType]
            for strip in range(len(shiftVals)):
                val = shiftVals[strip]
                if math.fabs(val) > 10:
                    b2.B2WARNING(f"cluster time shift ({val}) is very large for ({alg} : {sType} : size {strip + 1})")
            payload.setClusterTimeShift(alg, sType, shiftVals)

    # write out the payload to localdb directory
    Belle2.Database.Instance().storeData(Belle2.SVDClusterTimeShifter.name, payload, iov)
