#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import variables as v

# some variables should be there
assert len(v.variables.getVariables()) > 0

v.printVars()

var = v.variables.getVariable('M')
assert 'M' == var.name
print(var.description)

v.variables.addAlias('sigProb', 'extraInfo(SignalProbability)')
var = v.variables.getVariable('sigProb')
assert 'extraInfo(SignalProbability)' == var.name

assert (v.variables.evaluate('constant(123)', None) - 123) < 0.001

# used in FEI
import ROOT
assert 'extraInfo__boSignalProbability__bc' == ROOT.Belle2.Variable.makeROOTCompatible('extraInfo(SignalProbability)')
assert 'extraInfo(SignalProbability)' == ROOT.Belle2.Variable.invertMakeROOTCompatible('extraInfo__boSignalProbability__bc')
