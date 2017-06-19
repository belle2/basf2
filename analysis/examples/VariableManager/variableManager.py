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

import variables
from variables import variables as v

var = v.getVariable('M')
print("Name and Description of Variable M")
print(var.name)
print(var.description)

v.addAlias('sigProb', 'extraInfo(SignalProbability)')
var = v.getVariable('sigProb')
print("Real name of sigProb: ", var.name)

v.addCollection('Kinematics', variables.std_vector('px', 'py', 'pz'))
var = v.getCollection('Kinematics')
print("Collection named Kinematics: ", list(var))

result = v.evaluate('constant(123)', None)
print("Result of evaluating the variable 'constant(123)' ", result)

import ROOT
var = "extraInfo(SignalProbability)"
print("Root Compatble name of ", var, " is ", ROOT.Belle2.makeROOTCompatible(var))

print("You can get a full list of all available variables using 'basf2 variables.py'")
