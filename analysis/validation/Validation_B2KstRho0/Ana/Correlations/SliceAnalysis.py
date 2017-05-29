import ROOT
from ROOT import TFile, TCanvas, TH2F, TH1F
from ROOT import gROOT, gStyle
import os
import sys
import inspect
currentdir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
parentdir = os.path.dirname(currentdir)
sys.path.insert(0, parentdir)
from Global.fit_variables import fit_variables
from Global.samples import full_sample_name
from Global.selection_variables import *
from Global.cuts import *
from Global.paths import root_storage
from Decorations.Plotters import Plot_Hists_Together, Single_Variable_Plot
from Correlations.Correlations import Study_Fit_Variables_Correlations


def Define_Slices(file_name, tree_name, variable, nslices, cut, use_mc_limits=False, nbins=100):
    # This function defines equally-populated slices for the analysis
    f = TFile(file_name)
    tree = f.Get(tree_name)
    if use_mc_limits:
        try:
            llim = variable['MClLim']
            ulim = variable['MCuLim']
        except:
            llim = variable['lLim']
            ulim = variable['uLim']
    else:
        llim = variable['lLim']
        ulim = variable['uLim']
    h = TH1F("hist", ";" + variable['xaxis'] + ";", nbins, llim, ulim)
    tree.Project("hist", variable['tuple_name'], cut["cut"])
    print("N slices: " + str(nslices))
    nentries_in_slice = (h.GetEntries() - h.GetBinContent(0) - h.GetBinContent(nbins + 1)) / nslices
    print("Entries per slice: " + str(nentries_in_slice))
    entries = 0
    borders = [variable['lLim']]
    for b in range(1, nbins + 1):
        entries += h.GetBinContent(b)
        if entries >= nentries_in_slice:
            print("Border at bin " + str(b))
            borders.append(h.GetBinLowEdge(b))
            entries = 0
    borders.append(variable['uLim'])
    if len(borders) < nslices + 1:
        borders = Define_Slices(file_name, tree_name, variable, nslices, cut, use_mc_limits, nbins * 10)
    return borders


def Perform_Slice_Analysis_Of_Fit_Variables(file_name, tree_name, cut=no_cut):
    """
    This function plots angular analysis fit variables in slices of each other to detect correlations
    """
    gROOT.SetBatch()
    gStyle.SetOptStat(0)
    nslices = 4

    mode_name = file_name.split("/")[-1].split(".")[0] + "_" + tree_name
    p_address = "Plots/" + mode_name
    os.system("mkdir -p Plots/" + mode_name + "/TempPlots")
    os.system("mkdir -p " + p_address)

    hist_collection = {}
    for i, v in enumerate(fit_variables):
        if not (v in hist_collection.keys()):
            hist_collection[v] = {}
        if "MC" in tree_name:
            limits = Define_Slices(file_name, tree_name, fit_variables[v], nslices, cut, True)
        else:
            limits = Define_Slices(file_name, tree_name, fit_variables[v], nslices, cut, False)
        print("Number of slices: " + str(len(limits) - 1))
        for nslice in range(0, nslices):
            slice_cut = {"cut_name": v +
                         "_slice_" +
                         str(nslice +
                             1), "cut": "(" +
                         fit_variables[v]["tuple_name"] +
                         ">" +
                         str(limits[nslice]) +
                         ")&&(" +
                         fit_variables[v]["tuple_name"] +
                         "<=" +
                         str(limits[nslice +
                                    1]) +
                         ")"}
            print("SLice cut: " + slice_cut["cut"])
            if (cut["cut_name"] == "NC"):
                new_cut = slice_cut
            else:
                new_cut = cut_merger([cut, slice_cut])
            print("New cut: " + new_cut["cut"])
            for j, w in enumerate(fit_variables):
                h = Single_Variable_Plot(file_name, tree_name, fit_variables[w], "", new_cut, "Plots/" + mode_name + "/TempPlots/")
                if not (w in hist_collection[v].keys()):
                    hist_collection[v][w] = []
                hist_collection[v][w].append({"hist_addr": h, "name": "Slice " + str(nslice + 1) +
                                              " in " + fit_variables[v]["xaxis"].split(", ")[0]})
    for i, v in enumerate(fit_variables):
        for j, w in enumerate(fit_variables):
            if (v == w):
                Plot_Hists_Together(
                    hist_collection[v][w],
                    normalized=False,
                    name=w + "_in_" + v + "_slices",
                    leg_pos=[
                        0.0,
                        0.0,
                        1.0,
                        0.1],
                    add_legend=True,
                    address=p_address,
                    leg_columns=2)
            else:
                Plot_Hists_Together(
                    hist_collection[v][w],
                    normalized=True,
                    name=w + "_in_" + v + "_slices",
                    leg_pos=[
                        0.0,
                        0.0,
                        1.0,
                        0.1],
                    add_legend=True,
                    address=p_address,
                    leg_columns=2)
    Generate_report(file_name, tree_name)
    return True


def Generate_report(file_name, tree_name):
    mode_name = file_name.split("/")[-1].split(".")[0] + "_" + tree_name
    if not os.path.exists("Summaries"):
        os.system("mkdir Summaries")
    f = open("../Report/" + mode_name + ".tex", 'w')
    if "MC" in tree_name:
        level = "gen-"
        print("\\section{Generator-level correlations}", file=f)
        print("\\label{sec:GCorr}", file=f)
    else:
        level = "rec-"
        print("\\section{Resonctrucion-level correlations}", file=f)
        print("\\label{sec:RCorr}", file=f)

    if "MC" in tree_name:
        print("This section contains study of correlations between fit variables for the signal\
         sample at generation level. No cuts applied. Study is performed on mixed sample of \
         signal events with transverse and longituinal polarisation.", file=f)
    else:
        print("This section contains study of correlations between fit variables for the signal\
         sample at reconstruction level. All candidates used in the study are reconstructed \
         and truth-matched, no additional cuts applied. Study is performed on mixed sample of\
          signal events with transverse and longituinal polarisation.", file=f)
    print("\\input{Tab_" + mode_name + "_Correlation_Summary.tex}", file=f)
    for i, v in enumerate(fit_variables):
        print("\\begin{figure}[p]", file=f)
        print("\\centering", file=f)
        print("\\includegraphics[width=0.5\\textwidth]{../Correlations/Plots/" +
              mode_name + "/Merged_deltaE_in_" + v + "_slices.pdf}\\hfil", file=f)
        print("\\includegraphics[width=0.5\\textwidth]{../Correlations/Plots/" +
              mode_name + "/Merged_Ksthel_in_" + v + "_slices.pdf}\\\\", file=f)
        print("\\includegraphics[width=0.5\\textwidth]{../Correlations/Plots/" +
              mode_name + "/Merged_Mbc_in_" + v + "_slices.pdf}\\hfil", file=f)
        print("\\includegraphics[width=0.5\\textwidth]{../Correlations/Plots/" +
              mode_name + "/Merged_MKST_in_" + v + "_slices.pdf}\\\\", file=f)
        print("\\includegraphics[width=0.5\\textwidth]{../Correlations/Plots/" +
              mode_name + "/Merged_Mrho_in_" + v + "_slices.pdf}\\hfil", file=f)
        print("\\includegraphics[width=0.5\\textwidth]{../Correlations/Plots/" +
              mode_name + "/Merged_rhohel_in_" + v + "_slices.pdf}\\\\", file=f)
        print(
            "\\caption{Distributions of " +
            level +
            "level variables in regions of " +
            fit_variables[v]['tex_name'] +
            ".}",
            file=f)
        print("\\end{figure}\n", file=f)
    return True

if __name__ == "__main__":
    Study_Fit_Variables_Correlations(root_storage + "Bu_Rho0Kst+_K+pi0.root", "MCB2KstRho", no_cut)
    Perform_Slice_Analysis_Of_Fit_Variables(root_storage + "Bu_Rho0Kst+_K+pi0.root", "MCB2KstRho", no_cut)
    Study_Fit_Variables_Correlations(root_storage + "Bu_Rho0Kst+_K+pi0.root", "B2KstRho", cut_MCTrue)
    Perform_Slice_Analysis_Of_Fit_Variables(root_storage + "Bu_Rho0Kst+_K+pi0.root", "B2KstRho", cut_MCTrue)
    os.system("rm -rf Plots/*/TempPlots")
