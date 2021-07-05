##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
# !/usr/bin/env python3

"""
<header>
<noexecute>Used as library.</noexecute>
</header>
"""

import ROOT as R


def create1DHist(name, title, n_bins, x_min, x_max, x_label, y_label):
    h = R.TH1F(name, title, n_bins, x_min, x_max)
    h.GetXaxis().SetTitle(x_label)
    h.GetYaxis().SetTitle(y_label)
    return h


def addDetails(h, descr, check, contact_str, isShifter):
    h.GetListOfFunctions().Add(R.TNamed("Description", descr))
    h.GetListOfFunctions().Add(R.TNamed("Check", check))
    h.GetListOfFunctions().Add(R.TNamed("Contact", contact_str))
    if isShifter:
        h.GetListOfFunctions().Add(R.TNamed("MetaOptions", "shifter"))


# constants
SVDContact = "SVD Software Group, svd-software@belle2.org"


# selection of different parts of detector
cut_L3 = R.TCut('layer==3')
cut_L4 = R.TCut('layer==4')
cut_L5 = R.TCut('layer==5')
cut_L6 = R.TCut('layer==6')
cut_L456 = R.TCut('(layer==4)||(layer==5)||(layer==6)')
cut_s = R.TCut('sensor_type==0')  # slanted
cut_b = R.TCut('sensor_type==1')  # barrel
cut_U = R.TCut('strip_dir==0')  # U_P
cut_V = R.TCut('strip_dir==1')  # V_N
cut_noU = R.TCut('strip_dir!=0')  # V_P or -1
cut_noV = R.TCut('strip_dir!=1')  # U_N or -1
cut_size1 = R.TCut('(cluster_size==1)')
cut_size2 = R.TCut('(cluster_size==2)')
cut_size3plus = R.TCut('(cluster_size>2)')
cut_oneTH = R.TCut('cluster_truehits_number==1')  # one TrueHit associated with SVDCluster
cut_noUV = R.TCut('strip_dir==-1')  # no U, no V


# default granurality
gD = ((cut_L3 + cut_b + cut_U, 'L3_barrel_U_side'),
      (cut_L3 + cut_b + cut_V, 'L3_barrel_V_side'),
      (cut_L456 + cut_b + cut_U, 'L456_barrel_U_side'),
      (cut_L456 + cut_b + cut_V, 'L456_barrel_V_side'),
      (cut_L456 + cut_s + cut_U, 'L456_slanted_U_side'),
      (cut_L456 + cut_s + cut_V, 'L456_slanted_V_side'))

gD2 = ((cut_L3 + cut_b, 'L3_barrel'),
       (cut_L456 + cut_b, 'L456_barrel'),
       (cut_L456 + cut_s, 'L456_slanted'))

# granurality taking into account layers and type of sensor;
granulesLayersTypes = ((cut_L3 + cut_b, 'L3_barrel'),
                       (cut_L4 + cut_b, 'L4_barrel'),
                       (cut_L4 + cut_s, 'L4_slanted'),
                       (cut_L5 + cut_b, 'L5_barrel'),
                       (cut_L5 + cut_s, 'L5_slanted'),
                       (cut_L6 + cut_b, 'L6_barrel'),
                       (cut_L6 + cut_s, 'L6_slanted'))

# granularity for time differences between neighbour layers
granulesTD = ((cut_L3, 'L3-L4'),
              (cut_L4, 'L4-L5'),
              (cut_L5, 'L5-L6'))

granulesL3456 = ((cut_L3, 'L3456'),)  # characteristic of track saved in 3rd layer cluster

g_L3_V = ((cut_L3 + cut_V, 'L3_V'),)


def plotter(name, title, nbins, xmin, xmax, x_label, y_label,
            granules,
            tree, expr, cut,
            descr, check, contact_str=SVDContact, isShifter=False):
    for g in granules:
        hName = f'{name}_{g[1]}'
        h = create1DHist(hName, title, nbins, xmin, xmax, x_label, y_label)
        if cut == "":
            selection = g[0]
        else:
            selection = g[0] + cut
        tree.Draw(f'{expr}>>{hName}', selection, 'goff')
        addDetails(h, descr, check, contact_str, isShifter)
        h.SetTitle(f'{title} ({g[1]})')
        h.Write(hName)


def plotRegions(name, title, x_label, y_label,
                granules,
                tree, expr, cutALL, cut,
                descr, check, contact_str=SVDContact, isShifter=False):
    hName = f'{name}'
    h = create1DHist(hName, title, len(granules), 1, len(granules) + 1, x_label, y_label)
    h.GetYaxis().SetRangeUser(0, 1.4)
    for i, g in enumerate(granules, 1):
        h.GetXaxis().SetBinLabel(i, g[1])
        if cutALL == "":
            selectionALL = g[0]
        else:
            selectionALL = g[0] + cutALL
        n_all = tree.Draw(f'{expr}', selectionALL, 'goff')
        if cut == "":
            selection = g[0]
        else:
            selection = g[0] + cut
        n_selected = tree.Draw(f'{expr}', selectionALL + selection, 'goff')
        h.SetBinContent(i, n_selected / n_all)
        h.SetBinError(i, (n_selected / n_all) * (1 / n_selected + 1 / n_all)**0.5)
    addDetails(h, descr, check, contact_str, isShifter)
    h.SetTitle(f'{title}')
    h.Write(hName)
