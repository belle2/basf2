#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Adds default collections to the VariableManager
Details can be found on https://confluence.desy.de/display/BI/Physics+VariableManager

"""

import variables as v

# TODO add everything from analysis/NtupleTools/src/NtupleToolList.cc

v.variables.addCollection("Kinematics", v.std_vector("M", "p", "px", "py", "pz", "E"))
v.variables.addCollection("MCTruth", v.std_vector("isSignal", "mcErrors"))
