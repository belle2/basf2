#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Makes analysis variables (which can be used in many of the functions defined
in :mod:`modularAnalysis`) available to Python.
Details can be found on https://confluence.desy.de/display/BI/Physics+VariableManager
"""
from ROOT import gSystem
gSystem.Load('libanalysis.so')

# import everything into current namespace.
from ROOT import Belle2
variables = Belle2.Variable.Manager.Instance()

import ROOT


def std_vector(*args):
    """
    Creates an std::vector which can be passed to pyROOT
    """
    v = ROOT.std.vector(type(args[0]))()
    for x in args:
        v.push_back(x)
    return v
