#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import create_path, LogLevel, LogInfo, logging, set_random_seed, B2INFO, Module
import b2test_utils
from ROOT import Belle2, gRandom


class TestDoWhile(Module):
    """Small test module to print something in Path.doWhile"""

    def __init__(self):
        """Remember how many iterations we want"""
        super().__init__()
        #: use the event meta info to store the iterations
        self.eventInfo = Belle2.PyStoreObj("EventMetaData")

    def event(self):
        """Print the current iteration for this event as well as the first five random numbers"""
        weight = self.eventInfo.getGeneratedWeight()
        B2INFO(f"Current Iteration: {weight}")
        rndm = ["%.4f" % gRandom.Rndm() for i in range(5)]
        B2INFO(f"First 5 random numbers: {rndm}")
        weight += 1
        self.eventInfo.setGeneratedWeight(weight)
        self.return_value(weight > self.eventInfo.getRun())


env = Belle2.Environment.Instance()
# fixed random seed
set_random_seed("something important")
# simplify logging output to just the type and the message
for level in LogLevel.values.values():
    logging.set_info(level, LogInfo.LEVEL | LogInfo.MESSAGE)
# disable error summary, we don't need it for these short tests and it basically
# doubles the output
logging.enable_summary(False)

path = create_path()
path.add_module('EventInfoSetter', evtNumList=[2, 2, 1], runList=[0, 3, 5], expList=[0, 1, 3])
path.add_module('EventInfoPrinter')
subpath = create_path()
test_module = subpath.add_module(TestDoWhile())
path.do_while(subpath, max_iterations=3)
b2test_utils.safe_process(path)

# make sure that conditions on the last module are not accepted
test_module.if_true(create_path())
b2test_utils.run_in_subprocess(target=path.do_while, path=subpath)
# and also make sure empty paths are rejected
b2test_utils.run_in_subprocess(target=path.do_while, path=create_path())

# Try to loop over module not setting return value
path = create_path()
path.add_module('EventInfoSetter')
subpath = create_path()
subpath.add_module("EventInfoPrinter")
path.do_while(subpath)
b2test_utils.safe_process(path)

# Loop over a path with a condition leaving the loop should be rejected ... it
# just complicates things ...
path = create_path()
path.add_module("EventInfoSetter", evtNumList=5)
subpath = create_path()
subpath.add_module("EventInfoPrinter")
p1 = subpath.add_module("Prescale", prescale=0.9)
p1.if_true(create_path())
subpath.add_module("Prescale", prescale=0.2)
b2test_utils.run_in_subprocess(target=path.do_while, path=subpath)

# Do a sub path with a prescale module for looping
path = create_path()
path.add_module('EventInfoSetter', evtNumList=5)
subpath = create_path()
subpath.add_module("EventInfoPrinter")
subpath.add_module("Prescale", prescale=0.2)
path.do_while(subpath)
b2test_utils.safe_process(path)


with b2test_utils.clean_working_directory():
    # Ok, test pickling and loading the do_while(): Setting the PicklePath to a
    # non-empty value means that running process(path) will create the pickle file
    # and running process(None) will process the path in the pickle file
    env.setPicklePath("testpath.pkl")
    b2test_utils.safe_process(path)
    # lets add a new module just to make sure that the pickled path is executed,
    # not just the path we have here
    path.add_module("EventInfoPrinter")
    # and then load and execute the pickled path
    b2test_utils.safe_process(None)
    env.setPicklePath("")
