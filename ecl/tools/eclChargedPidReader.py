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
import subprocess
import argparse

parser = argparse.ArgumentParser(description=description)

parser.add_argument("inputpath",
                    metavar="inputpath",
                    type=str,
                    help="Path to the directory where input NTuples are stored.")

args = parser.parse_args()

import ROOT

# Silence ROOT!
ROOT.gROOT.SetBatch(True)

# ROOT.gROOT.LoadMacro("eclChargedPidSelector.C+g")

g_hypotheses = [11, 13, 211, 321, 2212]

if __name__ == "__main__":

    outputpath = "./HistosN1"
    print("Creating output directory for histograms:\n{0}".format(os.path.abspath(outputpath)))
    if not os.path.exists(outputpath):
        os.makedirs(outputpath)

    print("Start selector...")

    for hypo in g_hypotheses:
        print("Particle: {0}".format(hypo))
        input_p = ROOT.TFile.Open("{0}/pdg{1}.root".format(args.inputpath, hypo), "READ")
        tree_p = input_p.Get("n1_tree")
        tree_p.Process("eclChargedPidSelector.C+")
        input_p.Close()

        print("Antiparticle: {0}".format(hypo))
        input_ap = ROOT.TFile.Open("{0}/pdganti{1}.root".format(args.inputpath, hypo), "READ")
        tree_ap = input_ap.Get("n1_tree")
        tree_ap.Process("eclChargedPidSelector.C+")
        input_ap.Close()

    print("Done!")
