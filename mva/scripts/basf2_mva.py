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
    def __getattr__(self, name):
        return getattr(Belle2.MVA, name)

    def vector(self, *args):
        v = ROOT.std.vector(type(args[0]))()
        for x in args:
            v.push_back(x)
        return v

sys.modules[__name__] = ForwardBelle2MVANamespace()
