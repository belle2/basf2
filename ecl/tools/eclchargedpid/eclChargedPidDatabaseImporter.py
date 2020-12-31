# !/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import array
import argparse
import ROOT


description = """
eclChargedPidDatabaseImporter.py :
script to perform fits of PDFs for ECL charged PID for different charged particle hypotheses,
and create a (local) DB payload.
"""

__author__ = "Marco Milesi"
__email__ = "marco.milesi@unimelb.edu.au"
__maintainer__ = "Marco Milesi"
__date__ = "June 2018"

# NB: do NOT modify these lists, unless you really know what you're doing!
g_hypotheses = [11, -11, 13, -13, 211, -211, 321, -321, 2212, -2212]

parser = argparse.ArgumentParser(description=description)

parser.add_argument("inputpath",
                    metavar="inputpath",
                    type=str,
                    help="Path to the directory where input histograms are stored.")
parser.add_argument("--noDB",
                    dest="noDB",
                    action="store_true",
                    default=False,
                    help="Do not create DB payload. Just for debugging.")
parser.add_argument("-or", "--outputROOT",
                    dest="outputROOT",
                    action="store",
                    default="",
                    help="Create an output ROOT file with the PDFs at the path given. Useful for debugging.")
parser.add_argument("-op", "--outputplots",
                    dest="outputplots",
                    action="store",
                    default="",
                    help="Create plots for the fits in pdf format at the path given.")

args = parser.parse_args()


# Silence ROOT!
ROOT.gROOT.SetBatch(True)
ROOT.gErrorIgnoreLevel = ROOT.kWarning
# Silence RooFit!
ROOT.RooMsgService.instance().setGlobalKillBelow(ROOT.RooFit.WARNING)
ROOT.RooMsgService.instance().setSilentMode(True)

from electrons_pdf import fit_electron_eop  # noqa
from muons_pdf import fit_muon_eop  # noqa
from pions_pdf import fit_pion_eop  # noqa
from kaons_pdf import fit_kaon_eop  # noqa
from protons_pdf import fit_proton_eop  # noqa

if __name__ == "__main__":

    # Create a DB payload for ECL charged PID PDFs.
    payload = ROOT.Belle2.ECLChargedPidPDFs()

    # Create a 2D grid w/ the PDF binning to be stored in the DB payload.
    # row idx (vary along Y axis) : i --> p
    # column idx (vary along X axis) : j --> theta
    pmin_vals = [300.0, 400.0, 500.0, 750.0, 1000.0, 1500.0, 2000.0, 3000.0, 4000.0, 4500.0, 5000.0]
    thetamin_vals = [0.0, 17.0, 31.4, 32.2, 44.0, 117.0, 128.7, 130.7, 150.0]

    # MUST set the correct units in the payload object,
    # otherwise the module using the payload will have totally unexpected behaviour!
    payload.setEnergyUnit(ROOT.Belle2.Unit.MeV)
    payload.setAngularUnit(ROOT.Belle2.Unit.deg)

    arr_p = array.array("d", pmin_vals + [5500.0])
    arr_theta = array.array("d", thetamin_vals + [180.0])

    histgrid = ROOT.TH2F("binsgrid", "bins grid;#theta_{lab};p_{lab} [MeV/c]", len(thetamin_vals), arr_theta, len(pmin_vals), arr_p)
    # Fill w/ random stuff gaussianly-distributed. Just cosmetics.
    xyg = ROOT.TF2("xyg", "xygaus", thetamin_vals[0], 180.0, pmin_vals[0], 5500.0)
    xyg.SetParameters(10000, 75.0, 65.0, 2500.0, 2000.0)
    histgrid.FillRandom("xyg", 1000000)
    histgrid.SetDirectory(0)

    if args.outputROOT:
        if not os.path.exists(args.outputROOT):
            os.makedirs(args.outputROOT)

    for hypo in g_hypotheses:

        # Store the TH2F bin grid in the payload instance.
        payload.setBinsHist(hypo, histgrid)

        if args.outputROOT:
            append = "" if hypo > 0 else "anti"
            outROOT = ROOT.TFile("{0}/pdf_{1}{2}.root".format(args.outputROOT, append, abs(hypo)), "RECREATE")
            outROOT.cd()
            histgrid.Write()

        for ip, pmin in enumerate(pmin_vals, 1):

            # TEMP
            # if not ip==5: continue

            pmax = pmin_vals[ip] if ip <= pmin_vals.index(pmin_vals[-1]) else arr_p[-1]

            for jth, thetamin in enumerate(thetamin_vals, 1):

                # TEMP
                # if not jth==5: continue

                thetamax = thetamin_vals[jth] if jth <= thetamin_vals.index(thetamin_vals[-1]) else arr_theta[-1]

                params = {"inputpath": args.inputpath,
                          "pmin": pmin,
                          "pmax": pmax,
                          "idx_p": ip,
                          "thetamin": thetamin,
                          "thetamax": thetamax,
                          "idx_theta": jth,
                          "hypo": hypo,
                          "charge": hypo / abs(hypo),  # The fit functions need to know whether we are looking at +/- particles.
                          "outputplots": args.outputplots
                          }

                print(
                    "Hypothesis: {0},\t{1:.2f} < p < {2:.2f},\t{3:.2f} < theta < {4:.2f}".format(
                        hypo, pmin, pmax, thetamin, thetamax))

                if abs(hypo) == 11:
                    pdf = fit_electron_eop(**params)
                elif abs(hypo) == 13:
                    pdf = fit_muon_eop(**params)
                elif abs(hypo) == 211:
                    pdf = fit_pion_eop(**params)
                elif abs(hypo) == 321:
                    pdf = fit_kaon_eop(**params)
                elif abs(hypo) == 2212:
                    pdf = fit_proton_eop(**params)

                # Store the PDF for this 2D bin in the payload instance.
                payload.setPDFsInternalMap(hypo, ip, jth, pdf)

                if args.outputROOT:
                    outROOT.cd()
                    pdf.Write()

        payload.setPDFsMap(hypo)

    if not args.noDB:
        ROOT.Belle2.Database.Instance().storeData("ECLChargedPidPDFs",
                                                  payload,
                                                  ROOT.Belle2.IntervalOfValidity.always())
