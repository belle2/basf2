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

import variables as v

var = v.variables.getVariable('M')
print("Name and Description of Variable M")
print(var.name)
print(var.description)

v.variables.addAlias('sigProb', 'extraInfo(SignalProbability)')
var = v.variables.getVariable('sigProb')
print("Real name of sigProb: ", var.name)

result = v.variables.evaluate('constant(123)', None)
print("Result of evaluating the variable 'constant(123)' ", result)

import ROOT
var = "extraInfo(SignalProbability)"
print("Root Compatble name of ", var, " is ", ROOT.Belle2.makeROOTCompatible(var))

print("You can get a full list of all available variables using 'basf2 variables.py'")
