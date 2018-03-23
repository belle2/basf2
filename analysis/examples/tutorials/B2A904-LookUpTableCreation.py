#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from basf2 import *
from modularAnalysis import analysis_main
import random


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


bins_x = [make_1D_bin("P", 0, 2),
          make_1D_bin("P", 2, 3),
          make_1D_bin("P", 3, 4)]

bins_y = [make_1D_bin("PZ", 0, 1),
          make_1D_bin("PZ", 1, 5)]

tableIDNotSpec = []


random.seed()
for xbin in bins_x:
    for ybin in bins_y:
        weightInfo = {}
        weightInfo["Weight"] = float(random.randint(0, 100)) / 100
        weightInfo["StatErr"] = float(random.randint(100, 200)) / 100
        weightInfo["SystErr"] = 0
        tableIDNotSpec.append([weightInfo, make_2D_bin(xbin, ybin)])

outOfRangeWeightInfo = {}
outOfRangeWeightInfo["Weight"] = -1
outOfRangeWeightInfo["StatErr"] = -1
outOfRangeWeightInfo["SystErr"] = -1


print("Hi")

addtable = register_module('LookUpCreator')
addtable.param('tableIDNotSpec', tableIDNotSpec)
addtable.param('outOfRangeWeight', outOfRangeWeightInfo)
addtable.param('tableName', "ParticleReweighting:TestMomentum")

analysis_main.add_module(addtable)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10])
eventinfosetter.param('runList', [0])
eventinfosetter.param('expList', [0])
analysis_main.add_module(eventinfosetter)

process(analysis_main)
