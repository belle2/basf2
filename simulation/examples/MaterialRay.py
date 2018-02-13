#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import logging, LogLevel, create_path, process
import ROOT as root
import numpy as np
import matplotlib as mpl
mpl.use("Agg")
from matplotlib import pyplot as pl
from root_numpy import hist2array

# Don't show all the info messages
logging.log_level = LogLevel.ERROR

# create path
main = create_path()
# We need to process one event so we need the EventInfoSetter
main.add_module("EventInfoSetter", evtNumList=[1])
# We need the geometry parameters
main.add_module("Gearbox")
# as well as the geometry. We assign regions to each creator which allows to
# split the material budget by region instead of material.
geometry = main.add_module("Geometry", logLevel=LogLevel.INFO, assignRegions=True)
# MaterialScan uses the Geant4 setup which is created by the FullSim module so
# we need this as well
main.add_module('FullSim')
# And finally the MaterialScan module
materialscan = main.add_module("MaterialScan", logLevel=LogLevel.INFO)
materialscan.param({
    # Filename for output File
    'Filename': 'MaterialRay.root',
    'spherical': False,
    'planar': False,

    'ray': True,
    'ray.theta': 140,
    'ray.phi': 0,
    # Alternatively one can set the direction of the ray directly
    # 'ray.direction': [1, 0, 0],
    # Max depth for the scan in cm (x axis limit on the histogram). 0 = no limit
    'ray.maxDepth': 0,
    # Bin width (roughly) for the output histogram in cm
    'ray.sampleDepth': 0.1,
    # Opening angle for the rays: If <=0 only one ray is shot.  Otherwise all
    # rays will be distributed randomly in a cone with this opening angle in
    # degrees, distributed uniformly in the area (flat in cos(theta)
    'ray.opening': 0.05,
    # Number of rays to shoot if opening>0
    'ray.count': 1000,
})

# Do the MaterialScan, can take some time depending on the number of steps
process(main)

# Done, start plotting
rmaterial_file = root.TFile("MaterialRay.root")


def plot_hist(region, **argk):
    """Get the histogram for a given region from the file and plot it. Return the histogram data and edges"""
    h = rmaterial_file.Get("Ray/%s_x0" % region)
    if not h:
        return None
    data, edges = hist2array(h, return_edges=True)
    # Now we have to play around a bit: pl.plot(edges[:-1], data, drawstyle="steps-post") does work but
    # the last bin does not get a top line so we need to add a 0 to the
    # histogram to go back down to where we were
    data = np.append(data, 0)
    # now plot
    pl.plot(edges[0], data, drawstyle="steps-post", **argk)
    return data, edges[0]


# Get the all materials plot:
all_data, mat_edges = plot_hist("All_Regions", label="All regions", c="k")

# Now plot the different regions separately
for region in ["BeamPipe", "PXD", "SVD", "CDC", "ARICH", "TOP", "ECL", "EKLM", "BKLM", "STR", "COIL"]:
    plot_hist(region, label=region)

# Log scale anyone?
# pl.yscale("log")
pl.ylim(ymin=0)
pl.xlim(xmin=mat_edges[0], xmax=mat_edges[-1]+1)
pl.xlabel("Flight length [cm]")
pl.ylabel("Radiation length [$X_0 / %.3g$ cm]" % (mat_edges[1] - mat_edges[0]))
pl.legend(loc="best")
pl.tight_layout()
pl.savefig("MaterialRay.pdf")
