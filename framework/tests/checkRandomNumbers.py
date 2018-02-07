#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Check that random numbers don't change and are consistent.

This steering file runs through a set of exp/run combinations and prints
the first few random numbers in initialize(), beginRun(), event() and endRun().

It also sets the framework to debugging output and shows the information used
to calculate the random generator state.
"""

import struct
from basf2 import *
from ROOT import gRandom


def double_to_hex(double):
    """Convert a double to a hex string representation"""
    # we pack the float to a little-endian double end interpret it as
    # little-endian unsigned 64 bit int
    return "%016X" % struct.unpack("<Q", struct.pack("<d", double))[0]


class RandomTestModule(Module):
    """Print some random numbers to check reproducibility"""
    def __init__(self, name):
        """Make sure we can run in multiple processes"""
        super(RandomTestModule, self).__init__()
        self.set_property_flags(ModulePropFlags.PARALLELPROCESSINGCERTIFIED)
        #: save a name for the module to print it
        self.name = name

    def get_numbers(self, name):
        """Print the first 20 random numbers"""
        numbers = ["First 20 random values in %s::%s()" % (self.name, name)]
        for row in range(5):
            numbers.append(", ".join(double_to_hex(gRandom.Rndm()) for i in range(4)))
        B2INFO("\n  ".join(numbers))

    def initialize(self):
        """Show random numbers in initialize"""
        self.get_numbers("initialize")

    def beginRun(self):
        """Show random numbers in beginRun"""
        self.get_numbers("beginRun")

    def event(self):
        """Show random numbers in event"""
        self.get_numbers("event")

    def endRun(self):
        """Show random numbers in endRun"""
        self.get_numbers("endRun")


#: path used for processing
main = create_path()
# generate a few runs which look good in hex
main.add_module("EventInfoSetter", evtNumList=[3, 4, 5],
                runList=[0x11121314, 0x21222324, 0x31323334],
                expList=[0x123, 0x234, 0x345])
# We could add a single processing RandomBarrier to check random barrier
# transport in multi processing. But since output is unordered in multi
# processing we don't do this in the automatic test
# main.add_module("RandomBarrier").set_property_flags(0)
main.add_module("EventInfoPrinter")
main.add_module(RandomTestModule("test1"))
main.add_module("RandomBarrier")
main.add_module(RandomTestModule("test2"))

# now the libraries are loaded so we can set the loglevel to debug, set the seed
# and start processing
logging_framework = logging.package("framework")
logging_framework.set_log_level(LogLevel.DEBUG)
logging_framework.set_debug_level(200)
logging_framework.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)
set_random_seed("this is the seed")

process(main)
