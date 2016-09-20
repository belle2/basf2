#!/usr/bin/env python
# -*- coding: utf-8 -*-

#  ####### IMPORTANT! #######
# This file and the calibration_framework module are
# now out of date and left in for historical reasons
# only. They may be removed at any time.

# If you wish to use the current CAF please look at
# the other examples files.

import os
import sys
from basf2 import *
from calibration_framework import add_calibration, run_calibration
import ROOT
from ROOT.Belle2 import TestCalibrationAlgorithm

set_log_level(LogLevel.INFO)

# create a path with all modules needed for the collector
main = create_path()
main.add_module('EventInfoSetter',
                evtNumList=[50, 50, 100, 100, 100],
                expList=[1, 1, 1, 1, 1],
                runList=[1, 2, 3, 4, 5])

# add a calibration with the collectormodule "CaTest" and the
# TestCalibrationAlgorithm
add_calibration("CaTest", TestCalibrationAlgorithm())

# you can also create the collector and the algorithm before
# >>> collector = register_module("CaTest")
# >>> collector.param(...)
# >>> algorithm = TestCalibrationAlgorithm()
# >>> add_calibration(collector, algorithm)

# or give the calibration a name
# >>> add_calibration(collector, algorithm, "mycalibration")

# or provide a list of algorithms
# >>> add_calibration(collector, [algorithm1, algorithm2])

# run the calibration: this will add all calibrations to the main path and run
# it, including iterations
# run_calibration(main, max_iterations=5)

# you can specify an output directory for the calibration results or omit the
# max_iterations parameter
# >>> run_calibration(main, "calibration_dir")

# All iterations will have the same random seed so if your iteration criteria
# depends on randomness then this will not work. To get rid of this you can
# register a callback to be called before each iteration with one argument which
# is the iteration number


def before_iteration(iteration):
    B2RESULT("iteration callback for iteration %d" % iteration)
    set_random_seed("my seed for iteration %d" % iteration)

run_calibration(main, iteration_callback=before_iteration)
