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


bins_x = [make_1D_bin("xvar", -1, 0),
          make_1D_bin("xvar", 0, 1),
          make_1D_bin("xvar", 1, 2)]

bins_y = [make_1D_bin("yvar", -1, 0),
          make_1D_bin("yvar", 0, 1),
          make_1D_bin("yvar", 1, 2)]

tableIDNotSpec = []


random.seed()
for xbin in bins_x:
    for ybin in bins_y:
        weightInfo = {}
        weightInfo["infoA"] = float(random.randint(0, 100)) / 100
        weightInfo["infoB"] = -float(random.randint(0, 100)) / 100
        weightInfo["infoC"] = 0
        tableIDNotSpec.append([weightInfo, make_2D_bin(xbin, ybin)])

outOfRangeWeightInfo = {}
outOfRangeWeightInfo["infoA"] = -1
outOfRangeWeightInfo["infoB"] = -1
outOfRangeWeightInfo["infoC"] = -1


print("Hi")

addtable = register_module('LookUpCreator')
addtable.param('tableIDNotSpec', tableIDNotSpec)
addtable.param('outOfRangeWeight', outOfRangeWeightInfo)

analysis_main.add_module(addtable)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
analysis_main.add_module(eventinfosetter)

# analysis_main.add_module('Interactive')

process(analysis_main)
