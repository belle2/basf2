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

v.variables.addCollection("EventLevelTracking",
                          v.std_vector(
                              "nExtraCDCHits",
                              "nExtraCDCHitsPostCleaning",
                              "hasExtraCDCHitsInLayer(0)",
                              "hasExtraCDCHitsInLayer(1)",
                              "hasExtraCDCHitsInLayer(2)",
                              "hasExtraCDCHitsInLayer(3)",
                              "hasExtraCDCHitsInLayer(4)",
                              "hasExtraCDCHitsInLayer(5)",
                              "hasExtraCDCHitsInLayer(6)",
                              "hasExtraCDCHitsInLayer(7)",
                              "nExtraCDCSegments",
                              # "nExtraVXDHits",
                              # "nExtraVXDHitsInLayer(1)",
                              # "nExtraVXDHitsInLayer(2)",
                              # "nExtraVXDHitsInLayer(3)",
                              # "nExtraVXDHitsInLayer(4)",
                              # "nExtraVXDHitsInLayer(5)",
                              # "nExtraVXDHitsInLayer(6)",
                              # "svdFirstSampleTime",
                              "trackFindingFailureFlag"
                          ))
