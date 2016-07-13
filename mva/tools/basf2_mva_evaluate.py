#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2_mva
from basf2_mva_evaluation import roc_plot, diag_plot, distribution_plot
import argparse
import tempfile

import numpy as np
from B2Tools import b2latex
import ROOT
from ROOT import Belle2
from ROOT import gSystem
gSystem.Load('libanalysis.so')

import os
import shutil
import sys


def getCommandLineOptions():
    """ Parses the command line options of the fei and returns the corresponding arguments. """
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--identifiers', dest='identifiers', type=str, required=True, action='append', nargs='+',
                        help='DB Identifier or weightfile')
    parser.add_argument('-d', '--datafiles', dest='datafiles', type=str, required=True, action='append', nargs='+',
                        help='Data file containing ROOT TTree')
    parser.add_argument('-t', '--treename', dest='treename', type=str, default='tree', help='Treename in data file')
    parser.add_argument('-o', '--outputfile', dest='outputfile', type=str, default='output.pdf',
                        help='Name of the outputted pdf file')
    args = parser.parse_args()
    return args


def extract_names(identifiers, datafiles):
    """
    Extract information about an expert from the database using the given identifiers and datafile
    """
    probabilities = []
    truths = []
    variables = []
    labels = []
    for identifier in identifiers:
        general_options = basf2_mva.GeneralOptions()
        weightfile = basf2_mva.Weightfile.load(identifier)
        weightfile.getOptions(general_options)
        variables.append([Belle2.makeROOTCompatible(v) for v in general_options.m_variables])
        probabilities.append(Belle2.makeROOTCompatible(identifier))
        truths.append(Belle2.makeROOTCompatible(identifier + '_' + general_options.m_target_variable))
        labels.append(identifier)
    return probabilities, truths, variables, labels


if __name__ == '__main__':
    ROOT.gROOT.SetBatch(True)
    args = getCommandLineOptions()

    tempdir = tempfile.mkdtemp()
    identifiers = sum(args.identifiers, [])
    datafiles = sum(args.datafiles, [])

    rootfilename = tempdir + '/expert.root'
    basf2_mva.expert(basf2_mva.vector(*identifiers), basf2_mva.vector(*datafiles), args.treename, rootfilename)
    probabilities, truths, variables, labels = extract_names(identifiers)

    rootfile = ROOT.TFile(rootfilename, "UPDATE")

    # Change working directory after experts run, because they might want to access
    # a locadb in the current working directory
    old_cwd = os.getcwd()
    os.chdir(tempdir)

    o = b2latex.LatexFile()
    o += b2latex.TitlePage(title='Automatic MVA Evaluation',
                           authors=['Thomas Keck'],
                           abstract='Evaluation plots',
                           add_table_of_contents=True).finish()

    o += b2latex.Section("Plots")

    graphics = b2latex.Graphics()
    roc_plot.from_file(rootfile, probabilities, truths, labels, 'roc_plot.png')
    graphics.add('roc_plot.png', width=1.0)
    o += graphics.finish()

    graphics = b2latex.Graphics()
    diag_plot.from_file(rootfile, probabilities, truths, labels, 'diagonal_plot.png')
    graphics.add('diagonal_plot.png', width=1.0)
    o += graphics.finish()

    graphics = b2latex.Graphics()
    distribution_plot.from_file(rootfile, probabilities, truths, labels, 'distribution_plot.png')
    graphics.add('distribution_plot.png', width=1.0)
    o += graphics.finish()

    o.finish()
    o.save('latex.tex', compile=True)
    os.chdir(old_cwd)
    shutil.copy(tempdir + '/latex.pdf', args.outputfile)
