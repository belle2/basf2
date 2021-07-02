#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Thomas Keck 2016

import sys


from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True  # noqa

# FEI uses multi-threading for parallel execution of tasks therefore
# the ROOT gui-thread is disabled, which otherwise interferes sometimes
PyConfig.StartGuiThread = False  # noqa

# First load the required shared library
from ROOT import gSystem
from ROOT import Belle2
gSystem.Load('libmva.so')
gSystem.Load('libframework.so')
gSystem.Load('libanalysis.so')

# For some strange reason we need to do this to makeROOTCompatible be available.
Belle2.Variable.Manager
Belle2.Variable.Manager.Instance()
import ROOT  # noqa


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


Belle2.MVA.AbstractInterface.initSupportedInterfaces()
ROOT.gROOT.SetBatch(True)


# This is a hack (recommended by Guido) to replace this module
# with an object of the ForwardBelle2MVANamespace class
# Hence we are effectively able to implement getattr for a module.
sys.modules[__name__] = ForwardBelle2MVANamespace()
