import ROOT
from ROOT import TFile, TCanvas, TH2F
from ROOT import gROOT, gStyle
import os
import sys
import inspect
currentdir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
parentdir = os.path.dirname(currentdir)
sys.path.insert(0, parentdir)
from Global.fit_variables import fit_variables
from Global.selection_variables import *
from Global.cuts import *
from Global.paths import root_storage
import itertools


def Study_Fit_Variables_Correlations(file_name, tree_name, cut=no_cut):
    """
    This function calculates correlation matrix for fit variables and write it to the TeX table
    """

    gROOT.SetBatch()
    gStyle.SetOptStat(0)
    f = TFile(file_name)
    tree = f.Get(tree_name)
    mode_name = file_name.split("/")[-1].split(".")[0] + "_" + tree_name
    correlations = {}
    linearity = {}
    nBins = 100
    for pair in itertools.product(fit_variables.keys(), repeat=2):
        os.system("mkdir -p Plots/" + mode_name + "/" + cut["cut_name"])
        if "MC" in tree_name:
            try:
                llimX = fit_variables[pair[0]]['MClLim']
                ulimX = fit_variables[pair[0]]['MCuLim']
            except:
                llimX = fit_variables[pair[0]]['lLim']
                ulimX = fit_variables[pair[0]]['uLim']
            try:
                llimY = fit_variables[pair[1]]['MClLim']
                ulimY = fit_variables[pair[1]]['MCuLim']
            except:
                llimY = fit_variables[pair[1]]['lLim']
                ulimY = fit_variables[pair[1]]['uLim']
        else:
            llimX = fit_variables[pair[0]]['lLim']
            ulimX = fit_variables[pair[0]]['uLim']
            llimY = fit_variables[pair[1]]['lLim']
            ulimY = fit_variables[pair[1]]['uLim']
        h = TH2F(pair[0] + "_vs_" + pair[1], ";" + fit_variables[pair[0]]['xaxis'] + ";" + fit_variables[pair[1]]['xaxis'] + ";",
                 nBins, llimX, ulimX,
                 nBins, llimY, ulimY)
        tree.Project(pair[0] + "_vs_" + pair[1],
                     fit_variables[pair[1]]['tuple_name'] + ":" + fit_variables[pair[0]]['tuple_name'],
                     cut["cut"])
        c = TCanvas("c", "c", 900, 900)
        h.Draw("COLZ")
        c.SaveAs("Plots/" + mode_name + "/" + cut["cut_name"] + "/Corr_" + pair[0] + "_vs_" + pair[1] + ".pdf")

        correlations[pair[0] + "_vs_" + pair[1]] = h.GetCorrelationFactor()

    string0 = '{:10}'.format(' ')
    corr_strings = {}
    lin_strings = {}
    os.system("mkdir -p Summaries/" + mode_name + "/" + cut["cut_name"])
    f = open("../Report/Tab_" + mode_name + "_Correlation_Summary.tex", 'w')
    for varx in itertools.combinations(fit_variables.keys(), r=1):
        string0 += "&" + fit_variables[varx[0]]['tex_name']
        corr_strings[varx[0]] = fit_variables[varx[0]]['tex_name']
        lin_strings[varx[0]] = fit_variables[varx[0]]['tex_name']
        for vary in itertools.combinations(fit_variables.keys(), r=1):
            if varx[0] + "_vs_" + vary[0] in correlations.keys():
                if (correlations[varx[0] + "_vs_" + vary[0]] > 0.1):
                    corr_strings[varx[0]] += "&\\textbf{" + \
                        '{:{}{sign}{}.{}}'.format(correlations[varx[0] + "_vs_" + vary[0]], '^', 10, 2, sign='+') + \
                        "}"
                else:
                    corr_strings[varx[0]] += "&" + \
                        '{:{}{sign}{}.{}}'.format(correlations[varx[0] + "_vs_" + vary[0]], '^', 10, 2, sign='+')
            else:
                corr_strings[varx[0]] += "&" + '{:{align}{width}}'.format("---", align='^', width='10')

    print("Correlations:\n", file=f)
    print("\\begin{table}[h!]", file=f)
    print("\\centering", file=f)
    print("\\begin{tabular}{l|cccccc}", file=f)
    print(string0 + "\\\\", file=f)
    print("\\hline", file=f)
    for varx in itertools.combinations(fit_variables.keys(), r=1):
        print(corr_strings[varx[0]] + "\\\\", file=f)
    print("\\end{tabular}", file=f)
    if "MC" in tree_name:
        print("\\caption{Correlation matrix for gen-level sample.}", file=f)
    else:
        print("\\caption{Correlation matrix for rec-level sample.}", file=f)
    print("\\end{table}\n", file=f)
    return
