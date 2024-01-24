#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import numpy as np
import pandas as pd
import math
from pylab import savefig, subplot
import matplotlib.pyplot as plt
from matplotlib import ticker
import matplotlib as mpl
import sys


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
        text_file.write("{} {} {} {} {}\n".format(layer, ladder, sensor, param, value))
text_file.close()

# set to True, if automatic scaling is to be used, otherwise the values in the unit_scale dict will be used.
autoscale = True

# datafile structure
components = {'value'}
comp_map = {'value': 0}
parameters = {'du', 'dv', 'dw', 'alpha', 'beta', 'gamma', 'P_20', 'P_11', 'P_02', 'P_30', 'P_21', 'P_12', 'P_03'}
param_map = {1: 'du', 2: 'dv', 3: 'dw', 4: 'alpha', 5: 'beta', 6: 'gamma',
             31: 'P_20', 32: 'P_11', 33: 'P_02',
             41: 'P_30', 42: 'P_21', 43: 'P_12', 44: 'P_03'}
param_nums = {u: v for (v, u) in param_map.items()}
layers = {1, 2, 3, 4, 5, 6}

n_data = 10  # number of independent samples

# OK, we could make a dataframe for this, but then - why?
layer_phi0 = {
    1: 0.0,
    2: 0.0,
    3: 33.624 * np.pi / 180,
    4: 8 * np.pi / 180,
    5: -8 * np.pi / 180,
    6: -4 * np.pi / 180,
}
layer_nladders = {
    1: 8,
    2: 12,
    3: 7,
    4: 10,
    5: 12,
    6: 16,
}
layer_nsensors = {
    1: 2,
    2: 2,
    3: 2,
    4: 3,
    5: 4,
    6: 5,
}

# If you want to use preset scales, enter them here and set autoscale to False.
unit_scale = {'value': {'shift': (-10.0,
                                  10.,
                                  plt.cm.bwr,
                                  ' [um]'),
                        'rotation': (-1.0,
                                     1.0,
                                     plt.cm.bwr,
                                     ' [mrad]'),
                        'surface': (-10.0,
                                    10.,
                                    plt.cm.bwr,
                                    ' [um]')}}

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

# Scale things properly
scale = np.where((dataframe.parameter < 4) | (dataframe.parameter > 6), 1.0e4, 1.0e3)
for colname in ['value']:
    dataframe[colname] *= scale

# If bias is average of 10 samples, scale t_mille by sqrt(10)
# dataframe['t_mille'] *= np.sqrt(10)

# Calculate and print scales.
# Maxima can be calculated for series of columns
data_columns = ['value']
min_shift = dataframe[data_columns][dataframe.parameter < 4].min().min()
max_shift = dataframe[data_columns][dataframe.parameter < 4].max().max()
min_shift = min(min_shift, -max_shift)
max_shift = max(-min_shift, max_shift)
min_rot = dataframe[data_columns][(3 < dataframe.parameter) & (dataframe.parameter < 7)].min().min()
max_rot = dataframe[data_columns][(3 < dataframe.parameter) & (dataframe.parameter < 7)].max().max()
min_rot = min(min_rot, -max_rot)
max_rot = max(-min_rot, max_rot)
min_sur2 = dataframe[data_columns][(30 < dataframe.parameter) & (dataframe.parameter < 34)].min().min()
max_sur2 = dataframe[data_columns][(30 < dataframe.parameter) & (dataframe.parameter < 34)].max().max()
min_sur2 = min(min_sur2, -max_sur2)
max_sur2 = max(-min_sur2, max_sur2)
min_sur3 = dataframe[data_columns][(40 < dataframe.parameter) & (dataframe.parameter < 45)].min().min()
max_sur3 = dataframe[data_columns][(40 < dataframe.parameter) & (dataframe.parameter < 45)].max().max()
min_sur3 = min(min_sur3, -max_sur3)
max_sur3 = max(-min_sur3, max_sur3)

if math.isnan(min_rot):
    min_rot = 0.
if math.isnan(max_rot):
    max_rot = 0.
if math.isnan(min_shift):
    min_shift = 0.
if math.isnan(max_shift):
    max_shift = 0.
if math.isnan(min_sur2):
    min_sur2 = 0.
if math.isnan(max_sur2):
    max_sur2 = 0.
if math.isnan(min_sur3):
    min_sur3 = 0.
if math.isnan(max_sur3):
    max_sur3 = 0.

print('Symmetrized scales for value: ')
print('Shifts:    ' + str(min_shift) + ' to ' + str(max_shift))
print('Rotations: ' + str(min_rot) + ' to ' + str(max_rot))
print('Surface^2  ' + str(min_sur2) + ' to ' + str(max_sur2))
print('Surface^3  ' + str(min_sur3) + ' to ' + str(max_sur3))

if autoscale:
    unit_scale['value']['shift'] = (min_shift, max_shift, plt.cm.bwr, ' [um]')
    unit_scale['value']['rotation'] = (min_rot, max_rot, plt.cm.bwr, ' [mrad]')
    unit_scale['value']['surface2'] = (min_sur2, max_sur2, plt.cm.bwr, ' [um]')
    unit_scale['value']['surface3'] = (min_sur3, max_sur3, plt.cm.bwr, ' [um]')

fig = plt.figure(figsize=(10, 6.5))
for value in components:
    for shiftrot, i_params in zip(['shift', 'rotation'], [[1, 2, 3], [4, 5, 6]]):
        cmin, cmax, cmap, unit = unit_scale[value][shiftrot]
        for i_param in i_params:
            parameter = param_map[i_param]
            # Make a subframe
            val_param = dataframe[['layer', 'ladder', 'sensor', value]][dataframe['parameter'] == i_param]
            val_param[value] = val_param[value]
            # Here comes the real thing
            ax = subplot(2, 3, i_param, projection='polar')
            layer_gap = 24
            bottom = 24  # starts at this, increases by sensor height in layers and layer_gap in between.
            for layer in layers:
                ladder_width = 2 * np.pi / layer_nladders[layer]
                ladder_angles = np.linspace(layer_phi0[layer], layer_phi0[layer] + 2 * np.pi, layer_nladders[layer], endpoint=False)

                for sensor in range(1, 1 + layer_nsensors[layer]):
                    height = (layer > 3 and sensor == 1) * 10.0 + 28.0 / (1.0 + 0.3 * layer + 0.5 * sensor)
                    content = height * np.ones(layer_nladders[layer])
                    bars = ax.bar(
                        ladder_angles,
                        content,
                        width=ladder_width,
                        bottom=bottom,
                        label=str(sensor + 1),
                        align='center',
                        # edgecolor='k',
                    )
                    bottom += height
                    for (ladder, bar) in zip(range(1, 1 + layer_nladders[layer]), bars):
                        cond = (val_param.layer == layer) & (val_param.ladder == ladder) & (val_param.sensor == sensor)
                        try:
                            c = float(val_param[value][cond])
                        except BaseException:
                            c = 0.
                        if (cmax - cmin) > 0.:
                            color_value = (c - cmin) / (cmax - cmin)
                        else:
                            color_value = 0.
                        bar.set_facecolor(cmap(color_value))
                        bar.set_linewidth(0.2)

                label_r = bottom + 8
                for i in range(layer_nladders[layer]):
                    ax.text(
                        ladder_angles[i],
                        label_r,
                        str(i + 1),
                        ha='center',
                        va='center',
                        fontsize=6,
                        # color='k',
                    )
                bottom += layer_gap * (1 + 0.5 * (layer == 2))

            ax.grid(False)
            ax.axis('off')

            plt.title(parameter)

        # if we are done with shifts/rotations, collect some data
        # for the colorbar
        if shiftrot == 'shift':
            par_shifts = (cmap, cmin, cmax, unit)
        elif shiftrot == 'rotation':
            par_rots = (cmap, cmin, cmax, unit)

    # Plot colorbars after rings are in place
    axb1 = fig.add_axes([0.01, 0.55, 0.02, 0.33])
    cmap, cmin, cmax, unit = par_shifts
    norm1 = mpl.colors.Normalize(vmin=cmin, vmax=cmax)
    cb1 = mpl.colorbar.ColorbarBase(axb1, cmap=cmap, norm=norm1, orientation='vertical')
    cb1.set_label('Shifts ' + unit)
    cb1.locator = ticker.MaxNLocator(nbins=11)
    cb1.update_ticks()
    # rotations
    axb2 = fig.add_axes([0.01, 0.11, 0.02, 0.33])
    cmap, cmin, cmax, unit = par_rots
    norm2 = mpl.colors.Normalize(vmin=cmin, vmax=cmax)
    cb2 = mpl.colorbar.ColorbarBase(axb2, cmap=cmap, norm=norm2, orientation='vertical')
    cb2.set_label('Rotations ' + unit)
    cb2.locator = ticker.MaxNLocator(nbins=11)
    cb2.update_ticks()

    fig.suptitle(value, fontsize=20, x=0.01, y=0.95, horizontalalignment='left')
    figname = inputroot + '.rigid.png'
    savefig(figname)
    print('Saved figure ' + figname)

fig = plt.figure(figsize=(14, 6.5))
for value in components:
    for shiftrot, i_params in zip(['surface2', 'surface3'], [[31, 32, 33], [41, 42, 43, 44]]):
        cmin, cmax, cmap, unit = unit_scale[value][shiftrot]
        for i_param in i_params:
            parameter = param_map[i_param]
            # Make a subframe
            val_param = dataframe[['layer', 'ladder', 'sensor', value]][dataframe['parameter'] == i_param]
            val_param[value] = val_param[value]
            # Here comes the real thing
            if shiftrot == 'surface2':
                ax = subplot(2, 4, i_param-30, projection='polar')
            if shiftrot == 'surface3':
                ax = subplot(2, 4, i_param-36, projection='polar')
            layer_gap = 24
            bottom = 24  # starts at this, increases by sensor height in layers and layer_gap in between.
            for layer in layers:
                ladder_width = 2 * np.pi / layer_nladders[layer]
                ladder_angles = np.linspace(layer_phi0[layer], layer_phi0[layer] + 2 * np.pi, layer_nladders[layer], endpoint=False)

                for sensor in range(1, 1 + layer_nsensors[layer]):
                    height = (layer > 3 and sensor == 1) * 10.0 + 28.0 / (1.0 + 0.3 * layer + 0.5 * sensor)
                    content = height * np.ones(layer_nladders[layer])
                    bars = ax.bar(
                        ladder_angles,
                        content,
                        width=ladder_width,
                        bottom=bottom,
                        label=str(sensor + 1),
                        align='center',
                        # edgecolor='k',
                    )
                    bottom += height
                    for (ladder, bar) in zip(range(1, 1 + layer_nladders[layer]), bars):
                        cond = (val_param.layer == layer) & (val_param.ladder == ladder) & (val_param.sensor == sensor)
                        try:
                            c = float(val_param[value][cond])
                        except BaseException:
                            c = 0.
                        if (cmax - cmin) > 0.:
                            color_value = (c - cmin) / (cmax - cmin)
                        else:
                            color_value = 0.
                        bar.set_facecolor(cmap(color_value))
                        bar.set_linewidth(0.2)

                label_r = bottom + 8
                for i in range(layer_nladders[layer]):
                    ax.text(
                        ladder_angles[i],
                        label_r,
                        str(i + 1),
                        ha='center',
                        va='center',
                        fontsize=6,
                        color='k',
                    )
                bottom += layer_gap * (1 + 0.5 * (layer == 2))

            ax.grid(False)
            ax.axis('off')

            plt.title(parameter)

        # if we are done with shifts/rotations, collect some data
        # for the colorbar
        if shiftrot == 'surface2':
            par_sur2 = (cmap, cmin, cmax, unit)
        elif shiftrot == 'surface3':
            par_sur3 = (cmap, cmin, cmax, unit)

    # Plot colorbars after rings are in place
    axb1 = fig.add_axes([0.01, 0.55, 0.02, 0.33])
    cmap, cmin, cmax, unit = par_sur2
    norm1 = mpl.colors.Normalize(vmin=cmin, vmax=cmax)
    cb1 = mpl.colorbar.ColorbarBase(axb1, cmap=cmap, norm=norm1, orientation='vertical')
    cb1.set_label('Quadratic parameters ' + unit)
    cb1.locator = ticker.MaxNLocator(nbins=11)
    cb1.update_ticks()
    # rotations
    axb2 = fig.add_axes([0.01, 0.11, 0.02, 0.33])
    cmap, cmin, cmax, unit = par_sur3
    norm2 = mpl.colors.Normalize(vmin=cmin, vmax=cmax)
    cb2 = mpl.colorbar.ColorbarBase(axb2, cmap=cmap, norm=norm2, orientation='vertical')
    cb2.set_label('Cubic parameters ' + unit)
    cb2.locator = ticker.MaxNLocator(nbins=11)
    cb2.update_ticks()

    fig.suptitle(value, fontsize=20, x=0.01, y=0.95, horizontalalignment='left')

    figname = inputroot + '.surface.png'
    savefig(figname)
    print('Saved figure ' + figname)
