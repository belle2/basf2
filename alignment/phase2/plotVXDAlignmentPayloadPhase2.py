#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Created on Sun Feb  8 12:39:00 2015
Plot (mis)alignment of VXD at phase 2
@author: Jakub Kandra
"""
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

# fileName = 'errors.txt'

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
text_file.close()


beast2_sensors = [
    (1, 1, 1), (1, 1, 2),
    (2, 1, 1), (2, 1, 2),
    (3, 1, 1), (3, 1, 2),
    (4, 1, 1), (4, 1, 2), (4, 1, 3),
    (5, 1, 1), (5, 1, 2), (5, 1, 3), (5, 1, 4),
    (6, 1, 1), (6, 1, 2), (6, 1, 3), (6, 1, 4), (6, 1, 5)
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

fig = plt.figure(figsize=(8.27, 11.69))
ax = fig.add_subplot(111)

ax.text(0.00, 0.97, 'Experiment: 1002(Phase II), Run: XXXX, Date: 6/10/2017', color='black', size='14.5', weight='bold')

ax.text(0.05, 0.28, ' layer 3 \nladder 1', color='black', size='12', weight='bold')
ax.text(0.05, 0.15, ' layer 2 \nladder 1', color='black', size='12', weight='bold')
ax.text(0.05, 0.03, ' layer 1 \nladder 1', color='black', size='12', weight='bold')
ax.text(0.40, 0.53, ' layer 4 ladder 1', color='black', size='12', weight='bold')
ax.text(0.40, 0.73, ' layer 5 ladder 1', color='black', size='12', weight='bold')
ax.text(0.40, 0.93, ' layer 6 ladder 1', color='black', size='12', weight='bold')

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

sensorColor = [
    'green',
    'green',
    'green',
    'green',
    'green',
    'green',
    'green',
    'green',
    'green',
    'green',
    'green',
    'green',
    'green',
    'green',
    'green',
    'green',
    'green',
    'green']
excludedSensor = [True, True, True, True, True, True, True, True, True, True, True, True, True, True, True, True, True, True]
sensorAlpha = [0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1]
number = 1.00

quantity = [r'NAN', r'$u$ = ', r'$v$ = ', r'$w$ = ', r'$\alpha$ = ', r'$\beta$ = ', r'$\gamma$ = ']
unit = [r'NAN', ' um', ' um', ' um', ' mrad', ' mrad', ' mrad']
line = ['NAN', r'$u$ = NAN', r'$v$ = NAN', r'$w$ = NAN', r'$\alpha$ = NAN', r'$\beta$ = NAN', r'$\gamma$ = NAN']

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
            print('Non calculated sensor!!')
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

ax.axis('off')
fig.savefig('output.png', dpi=300)
plt.show()
