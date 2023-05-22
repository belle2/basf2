#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
#                                                                        #
# Usage: basf2 <script> -- -j ../../data/SVDTimeGrouping.json            #
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


def import_parameters_to_payload(payload, alg, numberOfAcquiredSamples):
    """ import parameters to payload """

    jsonVar = param["parsFor" + str(alg) + "In" + str(numberOfAcquiredSamples) + "Samples"]

    payload.setTimeGroupingParameters(alg, numberOfAcquiredSamples)._COMMENT = jsonVar["_COMMENT"]
    payload.setTimeGroupingParameters(alg, numberOfAcquiredSamples).tRange[0] = jsonVar["tRange"][0]
    payload.setTimeGroupingParameters(alg, numberOfAcquiredSamples).tRange[1] = jsonVar["tRange"][1]
    payload.setTimeGroupingParameters(alg, numberOfAcquiredSamples).rebinningFactor = jsonVar["rebinningFactor"]
    payload.setTimeGroupingParameters(alg, numberOfAcquiredSamples).fillSigmaN = jsonVar["fillSigmaN"]
    payload.setTimeGroupingParameters(alg, numberOfAcquiredSamples).limitSigma[0] = jsonVar["limitSigma"][0]
    payload.setTimeGroupingParameters(alg, numberOfAcquiredSamples).limitSigma[1] = jsonVar["limitSigma"][1]
    payload.setTimeGroupingParameters(alg, numberOfAcquiredSamples).fitRangeHalfWidth = jsonVar["fitRangeHalfWidth"]
    payload.setTimeGroupingParameters(alg, numberOfAcquiredSamples).removeSigmaN = jsonVar["removeSigmaN"]
    payload.setTimeGroupingParameters(alg, numberOfAcquiredSamples).fracThreshold = jsonVar["fracThreshold"]
    payload.setTimeGroupingParameters(alg, numberOfAcquiredSamples).maxGroups = jsonVar["maxGroups"]
    payload.setTimeGroupingParameters(alg, numberOfAcquiredSamples).expectedSignalTime[0] = jsonVar["expectedSignalTime"][0]
    payload.setTimeGroupingParameters(alg, numberOfAcquiredSamples).expectedSignalTime[1] = jsonVar["expectedSignalTime"][1]
    payload.setTimeGroupingParameters(alg, numberOfAcquiredSamples).expectedSignalTime[2] = jsonVar["expectedSignalTime"][2]
    payload.setTimeGroupingParameters(alg, numberOfAcquiredSamples).signalLifetime = jsonVar["signalLifetime"]
    payload.setTimeGroupingParameters(alg, numberOfAcquiredSamples).numberOfSignalGroups = jsonVar["numberOfSignalGroups"]
    payload.setTimeGroupingParameters(alg, numberOfAcquiredSamples).formSingleSignalGroup = jsonVar["formSingleSignalGroup"]
    payload.setTimeGroupingParameters(alg, numberOfAcquiredSamples).acceptSigmaN = jsonVar["acceptSigmaN"]
    payload.setTimeGroupingParameters(alg, numberOfAcquiredSamples).writeGroupInfo = jsonVar["writeGroupInfo"]
    payload.setTimeGroupingParameters(alg, numberOfAcquiredSamples).includeOutOfRangeClusters = jsonVar["includeOutOfRangeClusters"]
    sigmas = jsonVar["clsSigma"]
    for stype in range(len(sigmas)):
        for side in [0, 1]:     # 0:V, 1:U
            payload.setTimeGroupingParameters(alg, numberOfAcquiredSamples).clsSigma[stype][side].assign(sigmas[stype][side])


class timeGroupingConfigurationImporter(b2.Module):
    ''' importer of the SVDTimeGrouping Configuration'''

    def beginRun(self):
        '''begin run'''

        print("")
        print("--> json INFO:")
        print("              "+str(param["_COMMENT"]))
        print("")

        iov = Belle2.IntervalOfValidity.always()

        uniqueID = "SVDTimeGroupingConfiguration_" + str(now.isoformat())
        uniqueID += "_in" + param["uniqueID"]["INFO"]["source"]
        uniqueID += "_usedFor" + param["uniqueID"]["INFO"]["usedFor"]
        uniqueID += "_" + param["uniqueID"]["INFO"]["tag"]
        print("uniqueID ->", uniqueID)
        print("")

        payload = Belle2.SVDTimeGroupingConfiguration(uniqueID, param["_DESCRIPTION"])

        for alg in ["CoG3", "ELS3", "CoG6"]:
            for numberOfAcquiredSamples in [3, 6]:
                import_parameters_to_payload(payload, alg, numberOfAcquiredSamples)

        # write out the payload to localdb directory
        Belle2.Database.Instance().storeData(Belle2.SVDTimeGroupingConfiguration.name, payload, iov)


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

main.add_module(timeGroupingConfigurationImporter())

main.add_module('Progress')

b2.print_path(main)
b2.process(main)
print(b2.statistics)
