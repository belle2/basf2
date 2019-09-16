#!/usr/bin/env python3
# -*- coding: utf-8 -*-

description = """
This script loops over the particle hypotheses and feeds the NTuples
into the ROOT Selector for processing.  Within this processing, histograms for EoP, etc.
are filled based on the reconstructed momentum and theta bins.
These histograms are later used as input for the PDF fitter.
"""

__author__ = "Caitlin MacQueen, Marco Milesi"
__email__ = "cmq.centaurus@gmail.com, marco.milesi@unimelb.edu.au"
__date__ = "June 2018"

import os
import sys
import argparse

parser = argparse.ArgumentParser(description=description)

parser.add_argument("inputpath",
                    metavar="inputpath",
                    type=str,
                    help="Path to the directory where input NTuples are stored.")
parser.add_argument(
    "-o",
    "--outputpath",
    dest="outputpath",
    type=str,
    action="store",
    default=os.path.abspath(
        os.path.curdir) +
    "/HistosN1",
    help="Path to the output directory with the histograms for each particle hypothesis. Default is current directory.")

args = parser.parse_args()

import ROOT

# Silence ROOT!
ROOT.gROOT.SetBatch(True)

g_hypotheses = [11, 13, 211, 321, 2212]

if __name__ == "__main__":

    print("Creating output directory for histograms:\n{0}".format(os.path.abspath(args.outputpath)))
    if not os.path.exists(args.outputpath):
        os.makedirs(args.outputpath)

    print("Start selector...")

    selector = ROOT.TSelector.GetSelector("eclChargedPidSelector.C+")

    for hypo in g_hypotheses:

        print("Particle: {0}".format(hypo))
        input_p = ROOT.TFile.Open("{0}/pdg{1}.root".format(args.inputpath, hypo), "READ")
        selector.SetOutputDir(args.outputpath)
        tree_p = input_p.Get("n1_tree")
        tree_p.Process(selector)
        input_p.Close()

        print("Antiparticle: {0}".format(hypo))
        input_ap = ROOT.TFile.Open("{0}/pdganti{1}.root".format(args.inputpath, hypo), "READ")
        selector.SetOutputDir(args.outputpath)
        tree_ap = input_ap.Get("n1_tree")
        tree_ap.Process(selector)
        input_ap.Close()

    # Remove ROOT/ACliC by-products
    for ext in ["_C.d", "_C.so", "_C_ACLiC_dict_rdict.pcm"]:
        os.remove("{0}/eclChargedPidSelector{1}".format(os.path.abspath(os.path.curdir), ext))

    print("Done!")
