#!/usr/bin/env python3

#######################################################
#
# For photon energy bias correction
# This tutorial demonstrates how to create LooKUpTable and
# upload it to the database
#
######################################################

import basf2 as b2
import random
import pandas as pd

# Add some bin constructors


def make_1D_bin(name, min_val, max_val):
    return {name: [min_val, max_val]}


def get_weight(namew, w, namewerr, werr):
    return {namew: w, namewerr: werr}


def make_2D_bin(bin_x, bin_y):
    bin_2d = bin_x.copy()
    bin_2d.update(bin_y)
    return bin_2d


def make_3D_bin(bin_x, bin_y, bin_z):
    bin_3d = bin_x.copy()
    bin_3d.update(bin_y).update(bin_z)
    return bin_3d


# Bin IDs will be automatically assigned as follows:
#
#    E|0-2|2-3|3-4|
# theta|   |   |   |
#     -------------
#  0-1| 0 | 2 | 4 |
#  ---------------|
#  1-2| 1 | 3 | 5 |
#     -------------

weightsDB = '/nfs/dust/belle2/user/kovalch/energyBiasCorrFiles/Feb2021.txt'
df_weightDB = pd.read_csv(weightsDB, delimiter=r"\s+")

bins_x = []
bins_y = []
weightInfo = {}
weightInfo_x = []
tableIDNotSpec = []
binID = 0
tableIDSpec = []
for x in range(0, len(df_weightDB)):
    weightInfo = {}
    bins_x = make_1D_bin(df_weightDB['namebin1'].values[x], df_weightDB['minbin1'].values[x], df_weightDB['maxbin1'].values[x])
    bins_y = make_1D_bin(df_weightDB['namebin2'].values[x], df_weightDB['minbin2'].values[x], df_weightDB['maxbin2'].values[x])
    weightInfo = get_weight("Weight", df_weightDB['w'].values[x], "StatErr", df_weightDB['werr'].values[x])
    tableIDNotSpec.append([weightInfo, make_2D_bin(bins_x, bins_y)])
    binID += 1
    tableIDSpec.append([[weightInfo, make_2D_bin(bins_x, bins_y)], binID])

# And of course let's define out-of-range bin info
outOfRangeWeightInfo = {}
outOfRangeWeightInfo["Weight"] = -1
outOfRangeWeightInfo["StatErr"] = -1

# Now, let's configure table creator
addtable = b2.register_module('ParticleWeightingLookUpCreator')
addtable.param('tableIDSpec', tableIDSpec)
addtable.param('outOfRangeWeight', outOfRangeWeightInfo)
addtable.param('experimentHigh', -1)
addtable.param('experimentLow', 0)
addtable.param('runHigh', -1)
addtable.param('runLow', 0)
addtable.param('tableName', "Feb2021:TestEnergy")

addtable2 = b2.register_module('ParticleWeightingLookUpCreator')
addtable2.param('tableIDNotSpec', tableIDNotSpec)
addtable2.param('outOfRangeWeight', outOfRangeWeightInfo)
addtable2.param('experimentHigh', -1)
addtable2.param('experimentLow', 0)
addtable2.param('runHigh', -1)
addtable2.param('runLow', 0)
addtable2.param('tableName', "Feb2021:TestEnergy2")

eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10])
eventinfosetter.param('runList', [0])
eventinfosetter.param('expList', [0])

my_path = b2.create_path()
my_path.add_module(addtable)
my_path.add_module(addtable2)
my_path.add_module(eventinfosetter)

b2.process(my_path)
