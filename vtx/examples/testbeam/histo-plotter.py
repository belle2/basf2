##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
"""
Simple script for plotting testbeam data using fully reconstructed root files

All DUT specific adjustements can be made in the DUTConfig dictionary. The
plotter assumes pixel matrix with rectangular pixels.

Usage: python3 histo-plotter.py --ifile=VTXOutputDumper.root
"""

import ROOT
import os
import glob
from plots.residuals import plot as residualsPlot
from plots.residuals import make_pdf as residualsMakePdf
from plots.efficiency import plot as efficiencyPlot
from plots.inpixel import plot_superpixel as efficiencyPlotSuperpixel
from plots.efficiency import plot_super_inpix as efficiencyPlotSuperInpix

import argparse
parser = argparse.ArgumentParser(description="Perform plotting of test beam runs")
parser.add_argument('--ifile', dest='ifile', default='*', type=str, help='Input file pattern of runs to process')
args = parser.parse_args()


# Define ucell_axis & vcell_axis
u_axis_min = 0
v_axis_min = 0
u_axis_max = 450
v_axis_max = 3330

pitch_u = 0.04
pitch_v = 0.036

# Every plotting axis is given as a tuple (nbins,min,max)
DUTConfig = {'pitch_u':          pitch_u,              # in mm
             'pitch_v':          pitch_v,              # in mm
             'residual_u_axis':   (151, -0.1, +0.1),    # in mm
             'residual_v_axis':   (151, -0.1, +0.1),    # in mm
             'charge_unit':       'TOT',
             'seed_charge_axis':  (64, 0, 64),
             'clus_charge_axis':  (256, 0, 256),
             'ucell_axis':        (u_axis_max-u_axis_min, u_axis_min, u_axis_max),
             'vcell_axis':        (v_axis_max-v_axis_min, v_axis_min, v_axis_max),
             'sensor_u_axis':     (u_axis_max, -0.5*u_axis_max*pitch_u, 0.5*u_axis_max*pitch_u),
             'sensor_v_axis':     (v_axis_max, -0.5*v_axis_max*pitch_v, 0.5*v_axis_max*pitch_v),
             }


for inputfilename in glob.glob(args.ifile):

    # Open files with reconstructed run data
    inputfile = ROOT.TFile(inputfilename, 'READ')

    # Create one histofile per run
    histofile = ROOT.TFile('Plotter-' + os.path.basename(inputfilename), 'RECREATE', 'Histos created from file ' + inputfilename)

    # Add residual plots
    residualsPlot(inputfile, histofile, basecut="hasTrack==0", Config=DUTConfig)

    # Add efficiency plots
    efficiencyPlot(
        inputfile,
        histofile,
        basecut="",
        matchcut="hasHit==0",
        uaxis=(
            u_axis_max -
            u_axis_min,
            u_axis_min,
            u_axis_max),
        vaxis=(
            v_axis_max -
            v_axis_min,
            v_axis_min,
            v_axis_max))

    # Add superpixel in-pixel charge plots
    efficiencyPlotSuperpixel(
        inputfile,
        histofile,
        pixeltype=0,
        upitch=pitch_u,
        vpitch=pitch_v,
        ubins=20,
        vbins=20,
        ufold=2,
        vfold=2)

    # Add superpixel in-pixel efficiency plots
    efficiencyPlotSuperInpix(
        inputfile,
        histofile,
        basecut="",
        matchcut="hasHit==0",
        upitch=pitch_u,
        vpitch=pitch_v,
        ubins=20,
        vbins=20)

    # Make a pdf containing all plots
    pdfName = os.path.splitext(os.path.basename(inputfilename))[0] + '.pdf'
    residualsMakePdf(histofile, pdfName)

    # Close all files
    histofile.Write()
    histofile.Close()
    inputfile.Close()
