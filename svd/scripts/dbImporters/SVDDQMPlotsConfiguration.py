#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
# Usage: basf2 SVDDQMPlotsConfiguration.py -- -j ../../data/SVDDQMPlotsConfiguration.json
# Add shift values of other time algorithm in json file if needed.
##########################################################################


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

    uniqueID = "SVDDQMPlotsConfiguration_" + str(now.isoformat())
    uniqueID += "_in-" + param["uniqueID"]["INFO"]["source"]
    uniqueID += "_useFor-" + param["uniqueID"]["INFO"]["useFor"]
    uniqueID += "_" + param["uniqueID"]["INFO"]["tag"]
    print("uniqueID ->", uniqueID)
    print("")

    payload = Belle2.SVDDQMPlotsConfiguration(uniqueID)

    print("list of sensors:")
    for sensor in param["listOfSensors"]:
        payload.addSensorToList(sensor)
        print(" ", sensor)

    print("")
    print("3Samples:", param["3Samples"])

    if (param["3Samples"] == 1):
        payload.enablePlotsFor3SampleMonitoring()

    print("skipHLTRejectedEvents:", param["skipHLTRejectedEvents"])

    if (param["skipHLTRejectedEvents"] == 1):
        payload.enableSkipHLTRejectedEvents()

    # write out the payload to localdb directory
    Belle2.Database.Instance().storeData(Belle2.SVDDQMPlotsConfiguration.name, payload, iov)
