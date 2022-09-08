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
SVD MC Cluster Time Fudge Factor Formulas importer:
json files can be found in the svd_cdb_json repository (B2SVD).
"""
import basf2 as b2
from ROOT import Belle2
from basf2 import conditions as b2conditions
import datetime
import argparse
import json


def arg_parser():
    """ argument parser """

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-j', '--json',
                        type=str,
                        help='input json file',
                        metavar='JSON')

    return parser


def get_side(side):
    """returns the value of the side to be read in the json file"""

    if side == 0:
        return "V"
    else:
        return "U"


class jsonSVDMCClusterTimeFudgeFactorImporter(b2.Module):
    """
    Defining the python module to do the import.
    """

    def set_info_uniqueID(self, i1, i2, i3):
        self.first = i1
        self.current = i2
        self.tag = i3

    def beginRun(self):
        """
        call the functions to import the cluster parameters
        """
        iov = Belle2.IntervalOfValidity.always()

        uniqueID = "MCClusterTimeFudgeFactor_"+str(self.first)+"_" + str(
            now.isoformat()) + "_INFO:_current="+str(self.current)+"_"+str(self.tag)

        print("--> json INFO:")
        print("              "+str(param["_COMMENT"]))
        print("")

        cls_payload = Belle2.SVDMCClusterTimeFudgeFactor.t_payload(-1, str(uniqueID))

        geoCache = Belle2.VXD.GeoCache.getInstance()

        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    for side in (0, 1):

                        print("setting MC Cluster Time Fudge Factor for " +
                              str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber) + "." + str(side))

                        sigma = param[get_side(side)]["L"+str(layerNumber)]["S"+str(sensorNumber)]
                        print("sigma: "+str(sigma))

                        cls_payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), 1, sigma)

        Belle2.Database.Instance().storeData(Belle2.SVDMCClusterTimeFudgeFactor.name, cls_payload, iov)


if __name__ == "__main__":

    # Argument parsing
    args = arg_parser().parse_args()

    # get the time
    now = datetime.datetime.now()

    # open json file
    f = open(args.json)
    param = json.load(f)

    # Prepend the svd-only in geometry GT
    b2conditions.prepend_globaltag('svd_onlySVDinGeoConfiguration')

    main = b2.create_path()

    # Event info setter - execute single event
    eventinfosetter = b2.register_module('EventInfoSetter')
    eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
    main.add_module(eventinfosetter)

    main.add_module("Gearbox")
    main.add_module("Geometry")

    jsonImporter = b2.register_module(jsonSVDMCClusterTimeFudgeFactorImporter())
    jsonImporter.set_info_uniqueID(
        param["uniqueID"]["INFO"]["first"],
        param["uniqueID"]["INFO"]["current"],
        param["uniqueID"]["INFO"]["tag"])
    main.add_module(jsonImporter)

    # Show progress of processing
    progress = b2.register_module('Progress')
    main.add_module(progress)

    # Process events
    b2.process(main)
