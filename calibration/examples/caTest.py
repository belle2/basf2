#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
import sys
from basf2 import *
from ROOT import Belle2

set_log_level(LogLevel.INFO)
set_random_seed(13)


class PyAlgo(Module):

    def __init__(self, algorithm):
        """
        Initialize algorithm runner
        """
        super(PyAlgo, self).__init__()
        self.algo = algorithm

    def doFirstIteration(self, runlist):
        """
        Perfrom first iteration over list of runs
        """
        to_calibrate = []
        for_iteration = []

        lastRunToBeMerged = False
        while (len(runlist) or lastRunToBeMerged):
            if not len(to_calibrate) and len(runlist):
                to_calibrate.append(runlist[0])
                runlist.remove(runlist[0])

            if not len(to_calibrate):
                break

            def list_to_str(alist):
                res = ""
                for item in alist:
                    if res != "":
                        res = res + ","
                    res = res + str(item[0]) + "." + str(item[1])
                return res

            result = self.algo.execute(list_to_str(to_calibrate))
            if result == self.algo.c_OK:  # OK
                print("Calibration done for ", to_calibrate, " Sending constants to DB...")
                to_calibrate = []
                self.algo.commit()

            if result == self.algo.c_Iterate:  # Iterate
                print("Iteration requested for ", to_calibrate, " Sending constants to DB...")
                for_iteration.append(to_calibrate)
                to_calibrate = []
                self.algo.commit()

            if result == self.algo.c_NotEnoughData:  # Merge next
                print("Calibration for ", to_calibrate, " does not have enough data. Merging with next run if possible...")
                if len(runlist):
                    to_calibrate.append(runlist[0])
                    runlist.remove(runlist[0])
                if not len(runlist):
                    if not lastRunToBeMerged:
                        lastRunToBeMerged = True
                    else:
                        lastRunToBeMerged = False

            if result == self.algo.c_Failure:  # Fail
                print("Calibration failed for ", to_calibrate)
                to_calibrate = []

        return for_iteration

    def terminate(self):
        """
        reimplement Module::terminate()
        """
        print("Starting calibration...")

        print(self.algo.getDescription())
        raw_runlist = self.algo.getRunListFromAllData()
        runlist = []
        for exprun in raw_runlist:
            runlist.append((exprun.first, exprun.second))
        for_iter = self.doFirstIteration(runlist)
        print("Iteration needed for following data chunks: ", for_iter)
        import interactive
        # interactive.embed()


def addCalibration(path, algorithm, collector=None, name=None):
    """
    Add calibration to the path
    """
    if collector is None:
        collector = register_module(algo.getCollectorName())
    if name is not None:
        collector.param('prefix', name)
        algorithm.setPrefix(name)
    collector.param('granularity', 'run')
    path.add_module(collector)
    path.add_module(PyAlgo(algorithm))

# ----------------------------------------------------------------------------

main = create_path()
main.add_module('EventInfoSetter',
                evtNumList=[100, 100, 100, 100, 100],
                expList=[1, 1, 1, 1, 1],
                runList=[1, 2, 3, 4, 5])

algo = Belle2.calibration.TestCalibrationAlgorithm()
addCalibration(main, algo)

main.add_module('Progress')
main.add_module('RootOutput')

process(main)
print(statistics)

# ----------------------------------------------------------------------------

exit(0)

import interactive

print(" -----------------------------------------------------")
print(" You are in interactive environment")
print("")
print(" The algorithm can still access datastore and database")
print(" and you can do anything you like, e.g. execute it...")
print("")
print(" Exit with: [Ctrl]+[D] -> [y] -> [Enter]")
print("")
print(" algo = Belle2.calibration.TestCalibrationAlgorithm()")
print(" -----------------------------------------------------")

interactive.embed()
