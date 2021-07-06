#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import sys
# By default "import ROOT" parses the arguments *sigh* so the most convenient
# way to make sure that basf2 does not import ROOT is to add a --help and make
# sure the program does not stop there
from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = False  # noqa
sys.argv.append("--help")  # noqa

import basf2  # noqa
from basf2 import _constwrapper

# if the program is still alive make sure that the constants we use in basf2.py
# are still the same
PyConfig.IgnoreCommandLineOptions = True  # noqa
import ROOT
assert ROOT.kIsStatic == _constwrapper._ROOT_kIsStatic, "kIsStatic has changed its value"
assert ROOT.kIsConstMethod == _constwrapper._ROOT_kIsConstMethod, "kIsConstMethod has changed its value"
assert ROOT.kIsPublic == _constwrapper._ROOT_kIsPublic, "kIsPublic has changed its value"
