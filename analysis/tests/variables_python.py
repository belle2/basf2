#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from variables import printVars
from variables import std_vector
from variables import variables as v
# used in FEI
import ROOT

# some variables should be there
assert len(v.getVariables()) > 0

printVars()

var = v.getVariable('M')
assert 'M' == var.name
print(var.description)

v.addAlias('sigProb', 'extraInfo(SignalProbability)')
var = v.getVariable('sigProb')
assert 'extraInfo(SignalProbability)' == var.name

assert (v.evaluate('constant(123)', ROOT.nullptr) - 123) < 0.001

v.addAlias('anotherAlias', 'daughter(1, p)')

for vn in v.getAliasNames():
    assert (vn == 'sigProb' or vn == 'anotherAlias')

assert 'extraInfo__boSignalProbability__bc' == ROOT.Belle2.makeROOTCompatible('extraInfo(SignalProbability)')
assert 'extraInfo(SignalProbability)' == ROOT.Belle2.invertMakeROOTCompatible('extraInfo__boSignalProbability__bc')

v.addCollection('kin', std_vector('p', 'px', 'py', 'pz'))
vec = v.getCollection('kin')
assert vec[0] == 'p'
assert vec[1] == 'px'
assert vec[2] == 'py'
assert vec[3] == 'pz'
