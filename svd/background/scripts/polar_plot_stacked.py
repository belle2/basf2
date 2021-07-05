#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Alternative version with stacked bars
"""

import numpy as np
from ROOT import TFile
from pylab import plt, savefig, show, subplot

components = {'Touschek_LER', 'Touschek_HER'}
layers = {3, 4, 5, 6}

dose_data = {'Touschek_HER': {
    3: np.zeros((7, 2)),
    4: np.zeros((10, 3)),
    5: np.zeros((12, 4)),
    6: np.zeros((16, 5)),
}, 'Touschek_LER': {
    3: np.zeros((7, 2)),
    4: np.zeros((10, 3)),
    5: np.zeros((12, 4)),
    6: np.zeros((16, 5)),
}}

layer_phi0 = {
    3: 33.624 * np.pi / 180,
    4: 8 * np.pi / 180,
    5: -8 * np.pi / 180,
    6: -4 * np.pi / 180,
}
layer_nladders = {
    3: 7,
    4: 10,
    5: 12,
    6: 16,
}
layer_nsensors = {
    3: 2,
    4: 3,
    5: 4,
    6: 5,
}

fileName = '/data/belle2/BG/Feb2015/output/base_kekcc/base_touschek_histo.root'
print('reading from file ', fileName)

rootfile = TFile(fileName)
tree = rootfile.Get('bSummary')

nev = tree.GetEntries()
for iev in range(0, nev):
    tree.GetEntry(iev)

    dose_data[str(tree.component)][tree.layer][tree.ladder - 1, tree.sensor - 1] = 100 * tree.dose

overlap_factor = 0.0  # Overlap of neighbour bars relative to bar width

component = 'Touschek_LER'

for layer in layers:
    ladder_width = 2 * np.pi / layer_nladders[layer]
    ladder_angles = np.linspace(layer_phi0[layer], layer_phi0[layer] + 2 * np.pi,
                                layer_nladders[layer], endpoint=False)
    bottoms = np.zeros(layer_nladders[layer])
    ax = subplot(2, 2, layer - 2, polar=True)
    for sensor in range(layer_nsensors[layer]):
        bars = ax.bar(
            ladder_angles,
            dose_data[component][layer][:, sensor],
            width=ladder_width,
            bottom=bottoms,
            label=str(sensor + 1),
            align='center',
        )
        bottoms += dose_data[component][layer][:, sensor]
        for bar in bars:
            bar.set_facecolor(plt.cm.Paired(0.1 + 0.15 * sensor))
            bar.set_alpha(0.5)

    r_shift = 0.1 * max(bottoms)
    for (i, r) in zip(range(layer_nladders[layer]), bottoms):
        ax.text(
            ladder_angles[i],
            r + r_shift,
            str(i + 1),
            ha='center',
            va='center',
            fontsize=7,
            color='b',
        )

    for tick in ax.yaxis.get_major_ticks():
        tick.label.set_fontsize(7)

    for tick in ax.xaxis.get_major_ticks():
        tick.label.set_fontsize(9)

    fullscale = np.max(bottoms)
    ax.set_ylim(-0.25 * fullscale, 1.2 * fullscale)
    ax.text(
        1.1 * np.pi / 8,
        1.35 * np.max(bottoms),
        'Dose [Gy/smy]',
        ha='left',
        va='center',
        fontsize=7,
    )

    ax.legend(title='Sensor', fontsize=9, bbox_to_anchor=(1.4, 0.5))
    ax.set_title('Layer ' + str(layer), x=0.0, y=0.97)
    show()
savefig('layers.png')
