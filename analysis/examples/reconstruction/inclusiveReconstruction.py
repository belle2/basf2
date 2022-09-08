#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Doxygen should skip this script
# @cond

"""
An example script to reconstruct unspecified particles and the use of MC
matching tools for inclusive analyses with sum-of-exclusive method.
"""

import basf2
from modularAnalysis import inputMdst, reconstructDecay, matchMCTruth, \
    copyLists, variablesToNtuple
from variables.utils import create_aliases_for_selected
from variables import variables as vm  # shorthand for the variable manager instance
from stdCharged import stdE, stdK, stdPi
from stdPi0s import stdPi0s

mypath = basf2.create_path()
testinput = basf2.find_file('analysis/tests/mdst.root')
inputMdst(testinput, path=mypath)

stdPi0s("all", path=mypath)
stdE("loose", path=mypath)
stdK("loose", path=mypath)
stdPi("loose", path=mypath)

reconstructDecay("@Xsd:0 -> K+:loose pi-:loose", "", path=mypath)
reconstructDecay("@Xsd:1 -> K+:loose pi-:loose pi0:all", "", path=mypath)

copyLists("Xsd:comb", ["Xsd:0", "Xsd:1"], path=mypath)
# copyList("@Xsd:comb", ["Xsd:0", "Xsd:1"], path=mypath) # this can't work

reconstructDecay("B0:inclusive -> Xsd:comb e+:loose e-:loose",
                 "5.2 < Mbc < 5.3", path=mypath)

matchMCTruth("B0:inclusive", path=mypath)

interesting_variables = ['isSignal', "Mbc", "deltaE", "isUnspecified"]
interesting_variables += create_aliases_for_selected(
    ["px", "py", "pz", "E", "isSignal", "mcErrors", "isUnspecified"],
    "B0:inclusive -> ^Xsd:comb ^e+:loose ^e-:loose")

vm.printAliases()

variablesToNtuple("B0:inclusive", interesting_variables, path=mypath)

basf2.process(mypath)
print(basf2.statistics)

# @endcond
