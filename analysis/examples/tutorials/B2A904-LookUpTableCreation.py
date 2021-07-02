#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
#                                                                        #
# Stuck? Ask for help at questions.belle2.org                            #
#                                                                        #
# This tutorial demonstrates create LooKUpTable and                      #
# upload it to the database                                              #
#                                                                        #
##########################################################################

import basf2 as b2
import random

# Add some bin constructors


def make_1D_bin(name, min_val, max_val):
    return {name: [min_val, max_val]}


def make_2D_bin(bin_x, bin_y):
    bin_2d = bin_x.copy()
    bin_2d.update(bin_y)
    return bin_2d


def make_3D_bin(bin_x, bin_y, bin_z):
    bin_3d = bin_x.copy()
    bin_3d.update(bin_y).update(bin_z)
    return bin_3d


# Define bin ranges. Bins may be of different size
# They shouldn't event cover the whole parameter space
bins_x = [make_1D_bin("p", 0, 2),
          make_1D_bin("p", 2, 3),
          make_1D_bin("p", 3, 4)]

bins_y = [make_1D_bin("pz", 0, 1),
          make_1D_bin("pz", 1, 5)]


# Here would be input from the experts.
# We don't have real calibration tables yet
# So we use randomized info.
# Bin IDs will be automatically assigned as follows:
#
#    p|0-2|2-3|3-4|
#   pz|   |   |   |
#     -------------
#  0-1| 0 | 2 | 4 |
#  ---------------|
#  1-5| 1 | 3 | 5 |
#     -------------

tableIDNotSpec = []
random.seed()
for xbin in bins_x:
    for ybin in bins_y:
        weightInfo = {}
        weightInfo["Weight"] = float(random.randint(0, 100)) / 100
        weightInfo["StatErr"] = float(random.randint(100, 200)) / 100
        weightInfo["SystErr"] = float(10)
        tableIDNotSpec.append([weightInfo, make_2D_bin(xbin, ybin)])

# Now let's assign binIDs manually (to be able to assign identical IDs for different bins)
# Let's do this structure of bins (out-of-range bin -1 is assigned later):
#
#    p|0-2|2-3|3-4|
#   pz|   |   |   |
#     ---------------|
#     |              |
#     -------------  |
#  0-1| 43| 45|   |  |
#  ------------   |  |
#  1-5| 44|     46|  |
#     -------------  |
#     |            -1|
#     ---------------|
tableIDSpec = []
random.seed()
binID = 42
weightInfo = {}
for xbin in bins_x:
    for ybin in bins_y:
        if binID < 46:
            binID += 1
            weightInfo = {}
            weightInfo["Weight"] = float(random.randint(0, 100)) / 100
            weightInfo["StatErr"] = float(random.randint(100, 200)) / 100
            weightInfo["SystErr"] = float(10)
        tableIDSpec.append([[weightInfo, make_2D_bin(xbin, ybin)], binID])

# And of course let's define out-of-range bin info
outOfRangeWeightInfo = {}
outOfRangeWeightInfo["Weight"] = -1
outOfRangeWeightInfo["StatErr"] = -1
outOfRangeWeightInfo["SystErr"] = -1

# Now, let's configure table creator
addtable = b2.register_module('ParticleWeightingLookUpCreator')
addtable.param('tableIDSpec', tableIDSpec)
addtable.param('outOfRangeWeight', outOfRangeWeightInfo)
addtable.param('experimentHigh', 1000)
addtable.param('experimentLow', 0)
addtable.param('runHigh', 1000)
addtable.param('runLow', 0)
addtable.param('tableName', "ParticleReweighting:TestMomentum")

addtable2 = b2.register_module('ParticleWeightingLookUpCreator')
addtable2.param('tableIDNotSpec', tableIDNotSpec)
addtable2.param('outOfRangeWeight', outOfRangeWeightInfo)
addtable2.param('experimentHigh', 1000)
addtable2.param('experimentLow', 0)
addtable2.param('runHigh', 1000)
addtable2.param('runLow', 0)
addtable2.param('tableName', "ParticleReweighting:TestMomentum2")

eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10])
eventinfosetter.param('runList', [0])
eventinfosetter.param('expList', [0])

my_path = b2.create_path()
my_path.add_module(addtable)
my_path.add_module(addtable2)
my_path.add_module(eventinfosetter)

b2.process(my_path)
