#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# You can access the Variable Manager directly in Python
# This is useful either in PythonModules,
# or if you want to add Aliases for variables (so shorter names)
#
# Also it is important to know that the sometimes variables contain special characters like (,:
# Therefore we have to escape the variable names before putting them into ROOT files, you can get
# the branch-names outputted by modules like VariablesToHistogram, VariablesToNtuple and VariablesToTree
# using the makeROOTCompatible function
#
# Thomas Keck
#
# For full documentation please refer to https://software.belle2.org
# Anything unclear? Ask questions at https://questions.belle2.org

import variables
from variables import variables as vm  # shorthand name for the VariableManager instance

var = vm.getVariable('M')
print("Name and Description of Variable M")
print(var.name)
print(var.description)

vm.addAlias('sigProb', 'extraInfo(SignalProbability)')
var = vm.getVariable('sigProb')
print("Real name of sigProb: ", var.name)

vm.addCollection('Kinematics', variables.std_vector('px', 'py', 'pz'))
var = vm.getCollection('Kinematics')
print("Collection named Kinematics: ", list(var))

result = vm.evaluate('constant(123)', None)
print("Result of evaluating the variable 'constant(123)' ", result)

import ROOT
var = "extraInfo(SignalProbability)"
print("Root Compatble name of ", var, " is ", ROOT.Belle2.makeROOTCompatible(var))

print("You can get a full list of all available variables using 'basf2 variables.py'")
