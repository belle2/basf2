#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import B2FATAL, B2RESULT, Path, process, conditions, register_module
import b2test_utils
import modularAnalysis as ma
from variables import variables
import random
from ROOT import TFile

#######################################################
#
# Defining helper functions
#
######################################################

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


def check(ntupleName, treeName):
    """
    Verify results make sense.
    """
    ntuplefile = TFile(ntupleName)
    ntuple = ntuplefile.Get(treeName)

    if ntuple.GetEntries() == 0:
        B2FATAL("No pions saved")

    if not(ntuple.GetEntries("binID > 0 ") > 0):
        B2FATAL("Binning was applied incorrectly: no pions in physical bins")
    else:
        B2RESULT("Bins are defined")

    if not(ntuple.GetEntries("Weight > 0") > 0):
        B2FATAL("Weights are not applied")
    else:
        B2RESULT("Weights are applied")

#######################################################
#
# Creating mock calibration tables: one with
# user-defined bin IDs, another without them
#
######################################################


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

# we create payloads so let's switch to an empty, temporary directory
with b2test_utils.clean_working_directory():
    conditions.testing_payloads = ["localdb/database.txt"]

    # Now, let's configure table creator
    addtable = register_module('ParticleWeightingLookUpCreator')
    addtable.param('tableIDNotSpec', tableIDNotSpec)
    addtable.param('outOfRangeWeight', outOfRangeWeightInfo)
    addtable.param('experimentHigh', 1000)
    addtable.param('experimentLow', 0)
    addtable.param('runHigh', 1000)
    addtable.param('runLow', 0)
    addtable.param('tableName', "ParticleReweighting:TestMomentum")

    addtable2 = register_module('ParticleWeightingLookUpCreator')
    addtable2.param('tableIDSpec', tableIDSpec)
    addtable2.param('outOfRangeWeight', outOfRangeWeightInfo)
    addtable2.param('experimentHigh', 1000)
    addtable2.param('experimentLow', 0)
    addtable2.param('runHigh', 1000)
    addtable2.param('runLow', 0)
    addtable2.param('tableName', "ParticleReweighting:TestMomentum2")

    testpath = Path()
    testpath.add_module(addtable)
    testpath.add_module(addtable2)
    testpath.add_module('EventInfoSetter', evtNumList=[100], runList=[1], expList=[1])

    # Process the events to create database payloads
    process(testpath)
    B2RESULT("Weights are created and loaded to DB")

    # Now, let's test if it weights are applied
    main = Path()
    ntupleName = 'particleWeighting.root'
    treeName = 'pitree'
    inputfile = b2test_utils.require_file('analysis/tests/mdst.root')
    ma.inputMdst("default", inputfile, path=main)

    # use the MC truth information to generate pion lists
    ma.fillParticleListFromMC('pi+:gen', '', path=main)

    # ID of weight table is taken from B2A904
    weight_table_id = "ParticleReweighting:TestMomentum"

    # We know what weight info will be added (see B2A904),
    # so we add aliases and add it to tools
    variables.addAlias('Weight', 'extraInfo(' + weight_table_id + '_Weight)')
    variables.addAlias('StatErr', 'extraInfo(' + weight_table_id + '_StatErr)')
    variables.addAlias('SystErr', 'extraInfo(' + weight_table_id + '_SystErr)')
    variables.addAlias('binID', 'extraInfo(' + weight_table_id + '_binID)')
    varsPi = ['p', 'pz', 'Weight', 'StatErr', 'SystErr', 'binID']

    # We configure weighing module
    reweighter = register_module('ParticleWeighting')
    reweighter.param('tableName', weight_table_id)
    reweighter.param('particleList', 'pi+:gen')
    main.add_module(reweighter)

    # write out the flat ntuple
    ma.variablesToNtuple('pi+:gen', varsPi, filename=ntupleName, treename=treeName, path=main)

    # Process the events
    b2test_utils.safe_process(main)

    check(ntupleName, treeName)

B2RESULT("Weights were applied correctly")
