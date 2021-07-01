#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# The variableManager provides helpful tools for managing your variables.
#
# Contributors:
#   Thomas Keck
#   Kilian Lieret (2020)
#
# For full documentation please refer to https://software.belle2.org
# Anything unclear? Ask questions at https://questions.belle2.org

import ROOT
import variables
from variables import variables as vm  # shorthand name for the VariableManager instance

# Get information about a variable:
var = vm.getVariable('M')
print("Name and Description of Variable M")
print(var.name)
print(var.description)

# Also see variablesToExtraInfo.py
vm.addAlias('sigProb', 'extraInfo(SignalProbability)')
var = vm.getVariable('sigProb')
print("Real name of sigProb: ", var.name)

# We can also define collections of variables. For more on this, see usingCollections.py
vm.addCollection('Kinematics', variables.std_vector('px', 'py', 'pz'))
var = vm.getCollection('Kinematics')
print("Collection named Kinematics: ", list(var))

result = vm.evaluate('constant(123)', ROOT.nullptr)
print("Result of evaluating the variable 'constant(123)' ", result)

# the variable names can contain special characters like (,:
# Therefore we have to escape the variable names before putting them into ROOT files.
# You can get the branch-names outputted by modules like VariablesToHistogram, VariablesToNtuple and
# VariablesToTree using the makeROOTCompatible function
var = "extraInfo(SignalProbability)"
print("Root Compatible name of ", var, " is ", ROOT.Belle2.makeROOTCompatible(var))

print("You can get a full list of all available variables using 'basf2 variables.py'")
