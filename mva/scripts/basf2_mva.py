#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

import sys

# First load the required shared library
from ROOT import gSystem
gSystem.Load('libmva.so')
from ROOT import Belle2
import ROOT


class ForwardBelle2MVANamespace(object):
    """ Class which forwards lookups to the Belle2.MVA namespace,
        this makes the access to C++ functions/classes of the mva package more convenient """

    def __getattr__(self, name):
        """ Return attribute with the given name in the Belle2.MVA namespace """
        return getattr(Belle2.MVA, name)

    def vector(self, *args):
        """ Create an std::vector containing the passed args """
        v = ROOT.std.vector(type(args[0]))()
        for x in args:
            v.push_back(x)
        return v

# This is a hack (recommended by Guido) to replace this module
# with an object of the ForwardBelle2MVANamespace class
# Hence we are effectively able to implement getattr for a module.
sys.modules[__name__] = ForwardBelle2MVANamespace()
