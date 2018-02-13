#!/usr/bin/env python3
# -*- coding: utf-8 -*-


import math
from basf2 import *
import ROOT
from ROOT import Belle2


# set_log_level(LogLevel.ERROR)
# set some random seed
set_random_seed(10346)


class PXDTroubleCreator(Module):

    def event(self):

        clusters = Belle2.PyStoreArray("PXDClusters")
        digits = Belle2.PyStoreArray("PXDDigits")

        for cluster in clusters:
            if not self.check_sorting_ok(cluster):
                # this looks strangly, print the digits on problematic sensor
                for digit in digits:
                    if cluster.getSensorID() == digit.getSensorID():
                        B2INFO(
                            "DIGIT: sensorID={}  vID={}  uID={}".format(
                                digit.getSensorID(),
                                digit.getVCellID(),
                                digit.getUCellID()))

                # raise fatal error
                B2FATAL(
                    "Digits related to cluster are not properly sorted! sensorID={} and cluster shape name={} ".format(
                        cluster.getSensorID(),
                        self.get_full_shape_name(cluster)))

    def check_sorting_ok(self, cluster):
        digits = cluster.getRelationsTo("PXDDigits")
        is_ok = True
        prev_v = 0
        for digit in digits:
            if digit.getVCellID() < prev_v:
                is_ok = False
            prev_v = digit.getVCellID()
        return is_ok

    def get_full_shape_name(self, cluster):
        name = "F"
        digits = cluster.getRelationsTo("PXDDigits")
        for digit in digits:
            name += "D" + str(digit.getVCellID()) + '.' + str(digit.getUCellID())
        return name


if __name__ == "__main__":

    import glob
    import sys

    # Now let's create a path to simulate our events.
    main = create_path()
    main.add_module("EventInfoSetter", evtNumList=[1000])
    main.add_module("Gearbox")
    # We only need the pxd for this
    main.add_module("Geometry", components=['MagneticField', 'BeamPipe', 'PXD'], useDB=False)
    # main.add_module("Geometry",useDB=False)

    # Generate BBbar events
    main.add_module("EvtGenInput")

    main.add_module("FullSim")
    main.add_module("PXDDigitizer")
    main.add_module("PXDClusterizer")

    trouble = PXDTroubleCreator()
    main.add_module(trouble)
    main.add_module("Progress")

    process(main)
    print(statistics)
