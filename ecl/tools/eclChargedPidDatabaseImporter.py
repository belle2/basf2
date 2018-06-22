# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
eclChargedPidDatabaseImporter.py :
script to perform fits of PDFs for ECL charged PID for different charged particle hypotheses,
and create a (local) DB payload.
"""

__author__ = "Marco Milesi"
__email__ = "marco.milesi@unimelb.edu.au"
__maintainer__ = "Marco Milesi"

import os
import sys
import array
import glob
import argparse

# NB: do NOT modify these lists, unless you really know what you're doing!
g_hypotheses = [11, -11, 13, -13, 211, -211, 321, -321, 2212, -2212]

parser = argparse.ArgumentParser(
    description="script to perform fits of PDFs for ECL charged PID
    for different charged particle hypotheses,
    and create a(local) DB payload.")

parser.add_argument("inputpath", metavar="inputpath", type=str, help="Path to the directory where input histograms are stored.")
parser.add_argument(
    "outputpath",
    metavar="outputpath",
    type=str,
    help="The path where to store the fit output files. The chosen directory will be created if it does not exist yet.")
parser.add_argument("--plots", dest="plots", action="store_true", default=False, help="Create plots for the fits in pdf format.")

args = parser.parse_args()

import ROOT

# Silence ROOT!
ROOT.gROOT.SetBatch(True)

from electrons_pdf import fit_electron_eop
from muons_pdf import fit_muon_eop
from pions_pdf import fit_pion_eop
from kaons_pdf import fit_kaon_eop
from protons_pdf import fit_proton_eop

if __name__ == "__main__":

    if not os.path.exists(args.outputpath):
        os.makedirs(args.outputpath)

    # Create a DB payload for ECL charged PID PDFs.
    payload = ROOT.Belle2.ECLChargedPidPDFs()

    # Create a 2D grid w/ the PDF binning to be stored in the DB payload.
    # i_row (Y axis) : p
    # j_col (X axis) : theta
    pmin_vals = [300.0, 400.0, 500.0, 750.0, 1000.0,
                 1500.0, 2000.0, 3000.0, 4000.0, 4500.0, 5000.0]
    thetamin_vals = [0.0, 17.0, 31.4, 32.2, 44.0, 117.0, 128.7, 130.7, 150.0]

    payload.setEnergyUnit(ROOT.Belle2.Unit.MeV)
    payload.setAngularUnit(ROOT.Belle2.Unit.deg)

    arr_p = array.array("d", pmin_vals + [5500.0])
    arr_theta = array.array("d", thetamin_vals + [180.0])

    histgrid = ROOT.TH2F("binsgrid", "bins grid", len(thetamin_vals), arr_theta, len(pmin_vals), arr_p)
    histgrid.GetXaxis().SetTitle("#theta")
    histgrid.GetYaxis().SetTitle("P [MeV]")
    # Fill w/ random stuff gaussianly-distributed. Just cosmetics.
    xyg = ROOT.TF2("xyg", "xygaus", thetamin_vals[0], 180.0, pmin_vals[0], 5500.0)
    xyg.SetParameters(10000, 75.0, 65.0, 2500.0, 2000.0)
    histgrid.FillRandom("xyg", 1000000)
    histgrid.SetDirectory(0)

    for hypo in g_hypotheses:

        # Store the TH2F bin grid in the payload instance.
        payload.setBinsHist(hypo, histgrid)

        for i_pmin, pmin in enumerate(pmin_vals, 1):

            for j_thetamin, thetamin in enumerate(thetamin_vals, 1):

                params = {"inputpath": args.inputpath,
                          "outputpath": args.outputpath,
                          "pmin": pmin,
                          "idx_p": i_pmin,
                          "idx_theta": j_thetamin,
                          "hypo": hypo,
                          "charge": hypo / abs(hypo),  # The fit functions need to know whether we are looking at +/- particles.
                          "plots": args.plots
                          }

                if hypo == 11:
                    pdf = fit_electron_eop(**params)
                elif hypo == 13:
                    pdf = fit_muon_eop(**params)
                elif hypo == 211:
                    pdf = fit_pion_eop(**params)
                elif hypo == 321:
                    pdf = fit_kaon_eop(**params)
                elif hypo == 2212:
                    pdf = fit_proton_eop(**params)

                # Store the PDF for this 2D bin in the payload instance.
                payload.setPDFsMap(hypo, i_pmin, j_thetamin, pdf)

        ROOT.Belle2.Database.Instance().storeData("ECLChargedPidPDFs",
                                                  payload,
                                                  ROOT.Belle2.IntervalOfValidity.always())
