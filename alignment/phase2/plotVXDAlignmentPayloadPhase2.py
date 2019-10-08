#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# *****************************************************************************

# title           : plotVXDALignmentPayloadPhase2.py
# description     : Produce png file loaded from localdb for phase 2
# author          : Jakub Kandra (jakub.kandra@karlov.mff.cuni.cz)
# date            : 8. 2. 2018

# *****************************************************************************


import os
import sys
import numpy as np
import pandas as pd
import math
from pylab import *
import matplotlib.pyplot as plt
from matplotlib import ticker
import matplotlib as mpl
import matplotlib.patches as patches
from basf2 import *
import ROOT
from ROOT import Belle2

if len(sys.argv) < 2:
    sys.exit("No input .root file specified!")

inputroot = sys.argv[1]
file = ROOT.TFile(inputroot, "OPEN")
vxd = file.Get("VXDAlignment")

fileName = inputroot + '.txt'

text_file = open(fileName, "w")
text_file.write("layer ladder sensor param value\n")
for entry in vxd.getMap():
    element_parameter = entry.first
    value = entry.second
    element = element_parameter.first
    param = element_parameter.second
    vxdid = Belle2.VxdID(element)
    layer = vxdid.getLayerNumber()
    ladder = vxdid.getLadderNumber()
    sensor = vxdid.getSensorNumber()
    if sensor != 0:
        text_file.write("{0} {1} {2} {3} {4}\n".format(layer, ladder, sensor, param, value))
        # print("{0} {1} {2} {3} {4}\n".format(layer, ladder, sensor, param, value))

for parameter in range(1, 7):
    text_file.write("{0} {1} {2} {3} {4}\n".format(1, 0, 1, parameter, vxd.get(int(Belle2.VxdID(1, 0, 0, 1)), parameter)))
    text_file.write("{0} {1} {2} {3} {4}\n".format(1, 0, 2, parameter, vxd.get(int(Belle2.VxdID(1, 0, 0, 2)), parameter)))
    text_file.write("{0} {1} {2} {3} {4}\n".format(3, 0, 1, parameter, vxd.get(int(Belle2.VxdID(3, 0, 0, 1)), parameter)))
    text_file.write("{0} {1} {2} {3} {4}\n".format(3, 0, 2, parameter, vxd.get(int(Belle2.VxdID(3, 0, 0, 2)), parameter)))
    for layer in range(1, 7):
        text_file.write("{0} {1} {2} {3} {4}\n".format(layer, 1, 0, parameter, vxd.get(int(Belle2.VxdID(layer, 1, 0)), parameter)))

text_file.close()

beast2_sensors = [
    (1, 1, 1), (1, 1, 2),
    (2, 1, 1), (2, 1, 2),
    (3, 1, 1), (3, 1, 2),
    (4, 1, 1), (4, 1, 2), (4, 1, 3),
    (5, 1, 1), (5, 1, 2), (5, 1, 3), (5, 1, 4),
    (6, 1, 1), (6, 1, 2), (6, 1, 3), (6, 1, 4), (6, 1, 5)
]

beast2_ladders = [
    (1, 1, 0),
    (2, 1, 0),
    (3, 1, 0),
    (4, 1, 0),
    (5, 1, 0),
    (6, 1, 0)
]

beast2_halfshells = [
    (1, 0, 1),
    (3, 0, 1)
]

# Read data into a pandas DataFrame
print('reading from file ' + fileName)

dataframe = pd.read_table(
    fileName,
    sep=' ',
    skipinitialspace=True,
    skiprows=1,
    names=[
        'layer',
        'ladder',
        'sensor',
        'parameter',
        'value'])

scale = np.where(dataframe.parameter < 4, 1.0e4, 1.0e3)
for colname in ['value']:
    dataframe[colname] *= scale

font = {'family': 'normal', 'weight': 'bold', 'size': 22}

sensorCoordinates = (
    [0.20, 0.00, 0.60, 0.10],
    [0.60, 0.00, 0.99, 0.10],
    [0.20, 0.12, 0.60, 0.22],
    [0.60, 0.12, 0.99, 0.22],
    [0.20, 0.25, 0.60, 0.35],
    [0.60, 0.25, 0.99, 0.35],
    [0.00, 0.38, 0.30, 0.52],
    [0.30, 0.38, 0.60, 0.52],
    [0.60, 0.38, 0.90, 0.52],
    [0.00, 0.58, 0.25, 0.72],
    [0.25, 0.58, 0.50, 0.72],
    [0.50, 0.58, 0.75, 0.72],
    [0.75, 0.58, 0.99, 0.72],
    [0.00, 0.78, 0.20, 0.92],
    [0.20, 0.78, 0.40, 0.92],
    [0.40, 0.78, 0.60, 0.92],
    [0.60, 0.78, 0.80, 0.92],
    [0.80, 0.78, 0.99, 0.92])

excludedSensor = [True, True, True, True, True, True, True, True, True, True, True, True, True, True, True, True, True, True]
excludedLadder = [True, True, True, True, True, True]
excludedHalfShell = [True, True]

quantity = [r'NAN', r'$u$ = ', r'$v$ = ', r'$w$ = ', r'$\alpha$ = ', r'$\beta$ = ', r'$\gamma$ = ']
unit = [r'NAN', ' um', ' um', ' um', ' mrad', ' mrad', ' mrad']
line = ['NAN', r'$u$ = NAN', r'$v$ = NAN', r'$w$ = NAN', r'$\alpha$ = NAN', r'$\beta$ = NAN', r'$\gamma$ = NAN']

fig = plt.figure(figsize=(8.27, 11.69))
ax = fig.add_subplot(111)

ax.text(0.00, 0.97, 'Experiment: 1002(Phase II), Run: XXXX, Date: 6/10/2017', color='black', size='14.5', weight='bold')

ax.text(0.05, 0.03, ' layer 1 \nladder 1', color='black', size='12', weight='bold')
ax.text(0.05, 0.15, ' layer 2 \nladder 1', color='black', size='12', weight='bold')
ax.text(0.05, 0.28, ' layer 3 \nladder 1', color='black', size='12', weight='bold')
ax.text(0.40, 0.53, ' layer 4 ladder 1', color='black', size='12', weight='bold')
ax.text(0.40, 0.73, ' layer 5 ladder 1', color='black', size='12', weight='bold')
ax.text(0.40, 0.93, ' layer 6 ladder 1', color='black', size='12', weight='bold')


number = 0.0
for i in range(0, 18):
    layer_id, ladder_id, sensor_id = beast2_sensors[i]
    print('Sensor: ', layer_id, '.', ladder_id, '.', sensor_id)
    for j in [1, 2, 3, 4, 5, 6]:
        number = float(dataframe['value'][dataframe['layer'] == layer_id][dataframe['ladder'] == ladder_id]
                       [dataframe['sensor'] == sensor_id][dataframe['parameter'] == j])
        if (number != 0.0):
            excludedSensor[i] = False
            line[j] = quantity[j] + str(round(number, 5)) + unit[j]
            print(line[j])
        else:
            print('Non calculated ladder!!')
    print('')

    if excludedSensor[i]:
        if (i == 6 or i == 9 or i == 13):
            ax.add_patch(plt.Polygon([[sensorCoordinates[i][0],
                                       sensorCoordinates[i][1] - 0.03],
                                      [sensorCoordinates[i][2],
                                       sensorCoordinates[i][1]],
                                      [sensorCoordinates[i][2],
                                       sensorCoordinates[i][3]],
                                      [sensorCoordinates[i][0],
                                       sensorCoordinates[i][3] + 0.03]],
                                     facecolor='red',
                                     alpha=0.3,
                                     edgecolor='black'))
            ax.add_patch(plt.Polygon([[sensorCoordinates[i][0],
                                       sensorCoordinates[i][1] - 0.03],
                                      [sensorCoordinates[i][2],
                                       sensorCoordinates[i][1]],
                                      [sensorCoordinates[i][2],
                                       sensorCoordinates[i][3]],
                                      [sensorCoordinates[i][0],
                                       sensorCoordinates[i][3] + 0.03]],
                                     fill=None))
        else:
            ax.add_patch(plt.Polygon([[sensorCoordinates[i][0], sensorCoordinates[i][1]],
                                      [sensorCoordinates[i][2], sensorCoordinates[i][1]],
                                      [sensorCoordinates[i][2], sensorCoordinates[i][3]],
                                      [sensorCoordinates[i][0], sensorCoordinates[i][3]]],
                                     facecolor='red', alpha=0.3, edgecolor='black'))
            ax.add_patch(plt.Polygon([[sensorCoordinates[i][0], sensorCoordinates[i][1]],
                                      [sensorCoordinates[i][2], sensorCoordinates[i][1]],
                                      [sensorCoordinates[i][2], sensorCoordinates[i][3]],
                                      [sensorCoordinates[i][0], sensorCoordinates[i][3]]], fill=None))

    else:
        if (i == 6 or i == 9 or i == 13):
            ax.add_patch(plt.Polygon([[sensorCoordinates[i][0],
                                       sensorCoordinates[i][1] - 0.03],
                                      [sensorCoordinates[i][2],
                                       sensorCoordinates[i][1]],
                                      [sensorCoordinates[i][2],
                                       sensorCoordinates[i][3]],
                                      [sensorCoordinates[i][0],
                                       sensorCoordinates[i][3] + 0.03]],
                                     facecolor='green',
                                     alpha=0.1,
                                     edgecolor='black'))
            ax.add_patch(plt.Polygon([[sensorCoordinates[i][0],
                                       sensorCoordinates[i][1] - 0.03],
                                      [sensorCoordinates[i][2],
                                       sensorCoordinates[i][1]],
                                      [sensorCoordinates[i][2],
                                       sensorCoordinates[i][3]],
                                      [sensorCoordinates[i][0],
                                       sensorCoordinates[i][3] + 0.03]],
                                     fill=None))
        else:
            ax.add_patch(plt.Polygon([[sensorCoordinates[i][0],
                                       sensorCoordinates[i][1]],
                                      [sensorCoordinates[i][2],
                                       sensorCoordinates[i][1]],
                                      [sensorCoordinates[i][2],
                                       sensorCoordinates[i][3]],
                                      [sensorCoordinates[i][0],
                                       sensorCoordinates[i][3]]],
                                     facecolor='green',
                                     alpha=0.1,
                                     edgecolor='black'))
            ax.add_patch(plt.Polygon([[sensorCoordinates[i][0], sensorCoordinates[i][1]],
                                      [sensorCoordinates[i][2], sensorCoordinates[i][1]],
                                      [sensorCoordinates[i][2], sensorCoordinates[i][3]],
                                      [sensorCoordinates[i][0], sensorCoordinates[i][3]]], fill=None))

        if (i < 6):
            ax.text(
                sensorCoordinates[i][0] + 0.02,
                sensorCoordinates[i][1] + 0.080,
                line[1],
                color='black',
                size='8',
                bbox=dict(
                    facecolor='white',
                    edgecolor='black',
                    boxstyle='round,pad=0.15'))
            ax.text(
                sensorCoordinates[i][0] + 0.02,
                sensorCoordinates[i][1] + 0.045,
                line[2],
                color='black',
                size='8',
                bbox=dict(
                    facecolor='white',
                    edgecolor='black',
                    boxstyle='round,pad=0.15'))
            ax.text(
                sensorCoordinates[i][0] + 0.02,
                sensorCoordinates[i][1] + 0.010,
                line[3],
                color='black',
                size='8',
                bbox=dict(
                    facecolor='white',
                    edgecolor='black',
                    boxstyle='round,pad=0.15'))
            ax.text(
                sensorCoordinates[i][0] + 0.20,
                sensorCoordinates[i][1] + 0.078,
                line[4],
                color='black',
                size='8',
                bbox=dict(
                    facecolor='white',
                    edgecolor='black',
                    boxstyle='round,pad=0.15'))
            ax.text(
                sensorCoordinates[i][0] + 0.20,
                sensorCoordinates[i][1] + 0.045,
                line[5],
                color='black',
                size='8',
                bbox=dict(
                    facecolor='white',
                    edgecolor='black',
                    boxstyle='round,pad=0.15'))
            ax.text(
                sensorCoordinates[i][0] + 0.20,
                sensorCoordinates[i][1] + 0.012,
                line[6],
                color='black',
                size='8',
                bbox=dict(
                    facecolor='white',
                    edgecolor='black',
                    boxstyle='round,pad=0.15'))
        else:
            ax.text(sensorCoordinates[i][0] + (sensorCoordinates[i][2] - sensorCoordinates[i][0]) / 4 - 0.024,
                    sensorCoordinates[i][1] + 0.121,
                    line[1],
                    color='black',
                    size='8',
                    bbox=dict(facecolor='white',
                              edgecolor='black',
                              boxstyle='round,pad=0.15'))
            ax.text(sensorCoordinates[i][0] + (sensorCoordinates[i][2] - sensorCoordinates[i][0]) / 4 - 0.024,
                    sensorCoordinates[i][1] + 0.099,
                    line[2],
                    color='black',
                    size='8',
                    bbox=dict(facecolor='white',
                              edgecolor='black',
                              boxstyle='round,pad=0.15'))
            ax.text(sensorCoordinates[i][0] + (sensorCoordinates[i][2] - sensorCoordinates[i][0]) / 4 - 0.024,
                    sensorCoordinates[i][1] + 0.077,
                    line[3],
                    color='black',
                    size='8',
                    bbox=dict(facecolor='white',
                              edgecolor='black',
                              boxstyle='round,pad=0.15'))
            ax.text(sensorCoordinates[i][0] + (sensorCoordinates[i][2] - sensorCoordinates[i][0]) / 4 - 0.030,
                    sensorCoordinates[i][1] + 0.055,
                    line[4],
                    color='black',
                    size='8',
                    bbox=dict(facecolor='white',
                              edgecolor='black',
                              boxstyle='round,pad=0.15'))
            ax.text(sensorCoordinates[i][0] + (sensorCoordinates[i][2] - sensorCoordinates[i][0]) / 4 - 0.030,
                    sensorCoordinates[i][1] + 0.033,
                    line[5],
                    color='black',
                    size='8',
                    bbox=dict(facecolor='white',
                              edgecolor='black',
                              boxstyle='round,pad=0.15'))
            ax.text(sensorCoordinates[i][0] + (sensorCoordinates[i][2] - sensorCoordinates[i][0]) / 4 - 0.030,
                    sensorCoordinates[i][1] + 0.011,
                    line[6],
                    color='black',
                    size='8',
                    bbox=dict(facecolor='white',
                              edgecolor='black',
                              boxstyle='round,pad=0.15'))

    line = ['NAN', r'$u$ = NAN', r'$v$ = NAN', r'$w$ = NAN', r'$\alpha$ = NAN', r'$\beta$ = NAN', r'$\gamma$ = NAN']

ax.axis('off')
fig.savefig('outputSensors.png', dpi=300)

"""
fig = plt.figure(figsize=(8.27, 11.69))
ax = fig.add_subplot(111)
"""

ax.cla()
ax.text(0.00, 0.97, 'Experiment: 1002(Phase II), Run: XXXX, Date: 6/10/2017', color='black', size='14.5', weight='bold')

ax.text(0.00, 0.325, ' layer 1 ladder 1', color='black', size='12', weight='bold')
ax.text(0.00, 0.405, ' layer 2 ladder 1', color='black', size='12', weight='bold')
ax.text(0.00, 0.485, ' layer 3 ladder 1', color='black', size='12', weight='bold')
ax.text(0.00, 0.59, ' layer 4 ladder 1', color='black', size='12', weight='bold')
ax.text(0.00, 0.73, ' layer 5 ladder 1', color='black', size='12', weight='bold')
ax.text(0.00, 0.88, ' layer 6 ladder 1', color='black', size='12', weight='bold')

sensorCoordinates = (
    [0.20, 0.30, 0.60, 0.36],
    [0.60, 0.30, 0.99, 0.36],
    [0.20, 0.38, 0.60, 0.44],
    [0.60, 0.38, 0.99, 0.44],
    [0.20, 0.46, 0.60, 0.52],
    [0.60, 0.46, 0.99, 0.52],
    [0.00, 0.56, 0.30, 0.63],
    [0.30, 0.56, 0.60, 0.63],
    [0.60, 0.56, 0.90, 0.63],
    [0.00, 0.70, 0.25, 0.78],
    [0.25, 0.70, 0.50, 0.78],
    [0.50, 0.70, 0.75, 0.78],
    [0.75, 0.70, 0.99, 0.78],
    [0.00, 0.85, 0.20, 0.92],
    [0.20, 0.85, 0.40, 0.92],
    [0.40, 0.85, 0.60, 0.92],
    [0.60, 0.85, 0.80, 0.92],
    [0.80, 0.85, 0.99, 0.92])

number = 0.0
for i in range(0, 6):
    layer_id, ladder_id, sensor_id = beast2_ladders[i]
    for j in [1, 2, 3, 4, 5, 6]:
        number = float(dataframe['value'][dataframe['layer'] == layer_id][dataframe['ladder'] == ladder_id]
                       [dataframe['sensor'] == sensor_id][dataframe['parameter'] == j])
        if (number != 0.0):
            excludedLadder[i] = False

for i in range(0, 18):
    excludedSensor[i] = False
    if i in range(0, 2):
        if excludedLadder[0]:
            excludedSensor[i] = True
    if i in range(2, 4):
        if excludedLadder[1]:
            excludedSensor[i] = True
    if i in range(4, 6):
        if excludedLadder[2]:
            excludedSensor[i] = True
    if i in range(6, 9):
        if excludedLadder[3]:
            excludedSensor[i] = True
    if i in range(9, 13):
        if excludedLadder[4]:
            excludedSensor[i] = True
    if i in range(13, 18):
        if excludedLadder[5]:
            excludedSensor[i] = True

    if excludedSensor[i]:
        if (i == 6 or i == 9 or i == 13):
            ax.add_patch(plt.Polygon([[sensorCoordinates[i][0],
                                       sensorCoordinates[i][1] - 0.03],
                                      [sensorCoordinates[i][2],
                                       sensorCoordinates[i][1]],
                                      [sensorCoordinates[i][2],
                                       sensorCoordinates[i][3]],
                                      [sensorCoordinates[i][0],
                                       sensorCoordinates[i][3] + 0.03]],
                                     facecolor='red',
                                     alpha=0.3,
                                     edgecolor='black'))
            ax.add_patch(plt.Polygon([[sensorCoordinates[i][0],
                                       sensorCoordinates[i][1] - 0.03],
                                      [sensorCoordinates[i][2],
                                       sensorCoordinates[i][1]],
                                      [sensorCoordinates[i][2],
                                       sensorCoordinates[i][3]],
                                      [sensorCoordinates[i][0],
                                       sensorCoordinates[i][3] + 0.03]],
                                     fill=None))
        else:
            ax.add_patch(plt.Polygon([[sensorCoordinates[i][0], sensorCoordinates[i][1]],
                                      [sensorCoordinates[i][2], sensorCoordinates[i][1]],
                                      [sensorCoordinates[i][2], sensorCoordinates[i][3]],
                                      [sensorCoordinates[i][0], sensorCoordinates[i][3]]],
                                     facecolor='red', alpha=0.3, edgecolor='black'))
            ax.add_patch(plt.Polygon([[sensorCoordinates[i][0], sensorCoordinates[i][1]],
                                      [sensorCoordinates[i][2], sensorCoordinates[i][1]],
                                      [sensorCoordinates[i][2], sensorCoordinates[i][3]],
                                      [sensorCoordinates[i][0], sensorCoordinates[i][3]]], fill=None))

    else:
        if (i == 6 or i == 9 or i == 13):
            ax.add_patch(plt.Polygon([[sensorCoordinates[i][0],
                                       sensorCoordinates[i][1] - 0.03],
                                      [sensorCoordinates[i][2],
                                       sensorCoordinates[i][1]],
                                      [sensorCoordinates[i][2],
                                       sensorCoordinates[i][3]],
                                      [sensorCoordinates[i][0],
                                       sensorCoordinates[i][3] + 0.03]],
                                     facecolor='green',
                                     alpha=0.1,
                                     edgecolor='green'))

        else:

            ax.add_patch(plt.Polygon([[sensorCoordinates[i][0],
                                       sensorCoordinates[i][1]],
                                      [sensorCoordinates[i][2],
                                       sensorCoordinates[i][1]],
                                      [sensorCoordinates[i][2],
                                       sensorCoordinates[i][3]],
                                      [sensorCoordinates[i][0],
                                       sensorCoordinates[i][3]]],
                                     facecolor='green',
                                     alpha=0.1,
                                     edgecolor='green'))

ladderCoordinates = (
    [0.30, 0.325],
    [0.30, 0.405],
    [0.30, 0.485],
    [0.30, 0.59],
    [0.30, 0.74],
    [0.30, 0.88]
)

number = 0.0
for i in range(0, 6):
    layer_id, ladder_id, sensor_id = beast2_ladders[i]
    print('Sensor: ', layer_id, '.', ladder_id, '.', sensor_id)
    for j in [1, 2, 3, 4, 5, 6]:
        number = float(dataframe['value'][dataframe['layer'] == layer_id][dataframe['ladder'] == ladder_id]
                       [dataframe['sensor'] == sensor_id][dataframe['parameter'] == j])
        if (number != 0.0):
            line[j] = quantity[j] + str(round(number, 5)) + unit[j]
            print(line[j])
        else:
            print('Non calculated ladder!!')
    print('')

    if not excludedLadder[i]:
        ax.text(
            ladderCoordinates[i][0] + 0.000,
            ladderCoordinates[i][1] + 0.011,
            line[1],
            color='black',
            size='8',
            bbox=dict(
                facecolor='white',
                edgecolor='black',
                boxstyle='round,pad=0.15'))

        ax.text(
            ladderCoordinates[i][0] + 0.200,
            ladderCoordinates[i][1] + 0.011,
            line[2],
            color='black',
            size='8',
            bbox=dict(
                facecolor='white',
                edgecolor='black',
                boxstyle='round,pad=0.15'))
        ax.text(
            ladderCoordinates[i][0] + 0.400,
            ladderCoordinates[i][1] + 0.011,
            line[3],
            color='black',
            size='8',
            bbox=dict(
                facecolor='white',
                edgecolor='black',
                boxstyle='round,pad=0.15'))
        ax.text(
            ladderCoordinates[i][0] + 0.000,
            ladderCoordinates[i][1] - 0.011,
            line[4],
            color='black',
            size='8',
            bbox=dict(
                facecolor='white',
                edgecolor='black',
                boxstyle='round,pad=0.15'))
        ax.text(
            ladderCoordinates[i][0] + 0.200,
            ladderCoordinates[i][1] - 0.011,
            line[5],
            color='black',
            size='8',
            bbox=dict(
                facecolor='white',
                edgecolor='black',
                boxstyle='round,pad=0.15'))
        ax.text(
            ladderCoordinates[i][0] + 0.400,
            ladderCoordinates[i][1] - 0.011,
            line[6],
            color='black',
            size='8',
            bbox=dict(
                facecolor='white',
                edgecolor='black',
                boxstyle='round,pad=0.15'))
        line = ['NAN', r'$u$ = NAN', r'$v$ = NAN', r'$w$ = NAN', r'$\alpha$ = NAN', r'$\beta$ = NAN', r'$\gamma$ = NAN']

ax.text(0.10, 0.25, ' PXD half shell', color='black', size='12', weight='bold')
ax.text(0.70, 0.25, ' SVD half shell', color='black', size='12', weight='bold')

number = 0.0
for i in range(0, 2):
    layer_id, ladder_id, sensor_id = beast2_halfshells[i]
    print('Sensor: ', layer_id, '.', ladder_id, '.', sensor_id)
    for j in [1, 2, 3, 4, 5, 6]:
        number = float(dataframe['value'][dataframe['layer'] == layer_id][dataframe['ladder'] == ladder_id]
                       [dataframe['sensor'] == sensor_id][dataframe['parameter'] == j])
        if (number != 0.0):
            excludedHalfShell[i] = False
            line[j] = quantity[j] + str(round(number, 5)) + unit[j]
            print(line[j])
        else:
            print('Non calculated ladder!!')
    print('')

    if excludedHalfShell[i]:
        ax.add_patch(patches.Arc(xy=[0.45, 0.15], width=0.10, height=0.07, angle=.0,
                                 theta1=-90.0, theta2=90.0, color='red', alpha=0.1, linewidth=35.0))
        ax.add_patch(patches.Arc(xy=[0.45, 0.15], width=0.28, height=0.20, angle=.0,
                                 theta1=-90.0, theta2=90.0, color='red', alpha=0.1, linewidth=40.0))
    else:
        ax.add_patch(patches.Arc(xy=[0.45, 0.15], width=0.10, height=0.07, angle=.0,
                                 theta1=-90.0, theta2=90.0, color='green', alpha=0.1, linewidth=35.0))
        ax.add_patch(patches.Arc(xy=[0.45, 0.15], width=0.28, height=0.20, angle=.0,
                                 theta1=-90.0, theta2=90.0, color='green', alpha=0.1, linewidth=40.0))

        ax.text(0.12 + 0.6 * i, 0.22,
                line[1],
                color='black',
                size='8',
                bbox=dict(
                    facecolor='white',
                    edgecolor='black',
                    boxstyle='round,pad=0.15'))

        ax.text(0.12 + 0.6 * i, 0.19,
                line[2],
                color='black',
                size='8',
                bbox=dict(
                    facecolor='white',
                    edgecolor='black',
                    boxstyle='round,pad=0.15'))
        ax.text(0.12 + 0.6 * i, 0.16,
                line[3],
                color='black',
                size='8',
                bbox=dict(
                    facecolor='white',
                    edgecolor='black',
                    boxstyle='round,pad=0.15'))
        ax.text(0.12 + 0.6 * i, 0.13,
                line[4],
                color='black',
                size='8',
                bbox=dict(
                    facecolor='white',
                    edgecolor='black',
                    boxstyle='round,pad=0.15'))
        ax.text(0.12 + 0.6 * i, 0.10,
                line[5],
                color='black',
                size='8',
                bbox=dict(
                    facecolor='white',
                    edgecolor='black',
                    boxstyle='round,pad=0.15'))
        ax.text(0.12 + 0.6 * i, 0.07,
                line[6],
                color='black',
                size='8',
                bbox=dict(
                    facecolor='white',
                    edgecolor='black',
                    boxstyle='round,pad=0.15'))
        line = ['NAN', r'$u$ = NAN', r'$v$ = NAN', r'$w$ = NAN', r'$\alpha$ = NAN', r'$\beta$ = NAN', r'$\gamma$ = NAN']

ax.axis('off')
fig.savefig('outputLadders.png', dpi=300)
# plt.show()
