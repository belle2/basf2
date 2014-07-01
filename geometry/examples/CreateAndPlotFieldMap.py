#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
# This steering file creates the Belle II Magnetic field map and plots the
# result overlayed over the Material distribution
##############################################################################

import math
import os
from basf2 import *
import ROOT as root
import numpy as np

try:
    import matplotlib as mpl
    mpl.use("Agg")
except ImportError:
    print """\
Could not find matplotlib which is required to create the charts. Please
install it using
    pip install matplotlib"""
    sys.exit(1)

mpl.use("Agg")
from matplotlib import pyplot as pl
from matplotlib.backends.backend_pdf import PdfPages

##############################################################################
# Configuration
##############################################################################

scan_type = "zx"
stepsU = (1800, -400., 500.)
stepsV = (1600, -400., 400.)
fieldtype = "MagneticField3d"
filename = "fieldmap-%s.pdf" % scan_type
# if true, the magnetic field will be shown using a blue-white-red gradient,
# white being no field.
show_color = True
# if contours is defined and non empty, contour lines at the specified field
# strengths are drawn on top of the color map
show_contours = None
# colours for the chosen contours: can either be None, a single color name or a
# list of color names matching the chosen contours
contour_colors = 'k'

# example: show 1.4 to 1.6 tesla contours in blue, cyan, green, magenta, red
# show_contours = [1.4, 1.45, 1.5, 1.55, 1.6]
# contour_colors = ['b', 'c', 'g', 'm', 'r']

##############################################################################
# Create basf2 path to run fieldmap and material scan with compatible settings
##############################################################################

fieldmap_file = os.path.splitext(filename)[0] + "-fieldmap.root"
material_file = os.path.splitext(filename)[0] + "-material.root"

# set Materialscan plane to be consistent with magnetic field map
if scan_type == "xy":
    material_plane = [0, 0, -0.1, 1, 0, 0, 0, 1, 0]
elif scan_type == "zx":
    material_plane = [0, -0.1, 0, 0, 0, 1, 1, 0, 0]
elif scan_type == "zy":
    material_plane = [-0.1, 0, 0, 0, 0, 1, 0, 1, 0]

# create modules
evtinfo = register_module('EventInfoSetter')
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
simulation = register_module('FullSim')
# override field if we want something different than the default
if fieldtype != "MagneticField":
    geometry.param({
        "excludedComponents": ["MagneticField"],
        "additionalComponents": [fieldtype],
    })

# Field map creator
fieldmap = register_module('CreateFieldMap')
fieldmap.param({
    'filename': fieldmap_file,
    "type": scan_type,
    "nU": stepsU[0],
    "minU": stepsU[1],
    "maxU": stepsU[2],
    "nV": stepsV[0],
    "minV": stepsV[1],
    "maxV": stepsV[2],
})

# Material map creator
materialscan = register_module('MaterialScan')
materialscan.param({
    'Filename': material_file,
    'spherical': False,
    'planar': True,
    'planar.plane': 'custom',
    'planar.custom': material_plane,
    'planar.maxDepth': 0.2,
    'planar.nU': stepsU[0],
    'planar.minU': stepsU[1],
    'planar.maxU': stepsU[2],
    'planar.nV': stepsV[0],
    'planar.minV': stepsV[1],
    'planar.maxV': stepsV[2],
    'planar.splitByMaterials': False,
    'planar.ignored': ['Air', 'Vacuum', 'G4_AIR', 'ColdAir'],
})

# Create main path
main = create_path()

# Add modules to main path
main.add_module(evtinfo)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(simulation)
main.add_module(fieldmap)
main.add_module(materialscan)

# Process one event to perform the scans
process(main)

##############################################################################
# Now we have the files and create the plots
##############################################################################


def get_hist_data(hist):
    """Obtain the data and extent of a ROOT TH2 as a numpy array"""
    nbinsX = hist.GetNbinsX()
    nbinsY = hist.GetNbinsY()
    data = np.frombuffer(hist.GetArray(), count=hist.GetSize())
    # rehsape and strip overflow/underflow bins
    data = data.reshape(nbinsY + 2, nbinsX + 2)[1:-1, 1:-1]

    xmin = hist.GetXaxis().GetXmin()
    xmax = hist.GetXaxis().GetXmax()
    ymin = hist.GetYaxis().GetXmin()
    ymax = hist.GetYaxis().GetXmax()

    return data, (xmin, xmax, ymin, ymax)


def plotMap(axes, data, extent, cmap, **kwargs):
    """Plot either material or field map"""
    return axes.imshow(data, extent=extent, origin="lower", cmap=cmap,
                       aspect="auto", interpolation="nearest", **kwargs)


# open root files we just created
rfieldmap_file = root.TFile(fieldmap_file)
rmaterial_file = root.TFile(material_file)
# get the material map
mat_hist = rmaterial_file.Get("Planar/All_Regions_x0")
mat_data, mat_extent = get_hist_data(mat_hist)
# obtain nice colormaps
cmap_fieldmap = mpl.cm.get_cmap('seismic')
cmap_material = mpl.cm.get_cmap('binary')
# create pdf file and fill with plots
pdf = PdfPages(filename)
for component in ["Bx", "By", "Bz", "Br"]:
    print "Create %s fieldmap plot" % component
    h = rfieldmap_file.Get(component)
    data, extent = get_hist_data(h)
    # we need the scale to be symmetric so find out what is the maximal
    # absolute value and we choose a minimum scale
    field_max = max(np.abs(data).max(), 0.1)
    f = pl.figure()
    a = f.add_subplot(111)
    # plot material
    lognorm = mpl.colors.LogNorm()
    plotMap(a, mat_data, mat_extent, cmap_material, norm=lognorm)
    # and overlay magnetic field
    if show_color:
        m = plotMap(a, data, extent, cmap_fieldmap, alpha=0.9,
                    vmin=-field_max, vmax=field_max)
        # add a colorbar and set the label
        cb = f.colorbar(m)
        cb.set_label(r"$B_%s/{\rm T}$" % component[1])

    if show_contours is not None and len(show_contours) > 0:
        # get bin centers in x and y
        x, xstep = np.linspace(extent[0], extent[1], h.GetNbinsX() + 1,
                               retstep=True)
        x += xstep / 2
        y, ystep = np.linspace(extent[2], extent[3], h.GetNbinsY() + 1,
                               retstep=True)
        y += ystep / 2
        x, y = np.meshgrid(x[:-1], y[:-1])
        # draw contour plot and label it
        cs = a.contour(x, y, data, levels=show_contours, colors=contour_colors)
        a.clabel(cs, fmt="%.2f T", fontsize=6)

    #label the plot
    a.set_title(h.GetTitle())
    a.set_xlabel(r"$%s/{\rm cm}$" % scan_type[0])
    a.set_ylabel(r"$%s/{\rm cm}$" % scan_type[1])
    # findally, save the plot
    pdf.savefig(f)

pdf.close()
