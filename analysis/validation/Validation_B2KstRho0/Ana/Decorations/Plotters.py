"""
This file contains collection of helper functions used for making nice plots.
"""

import ROOT
from ROOT import TFile, TCanvas, TH1F, TH2F, TGaxis, TStyle, TProfile
from ROOT import gROOT, gStyle, gPad
import os
import sys
import inspect
#: get the current directory
currentdir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
#: get the parent directory
parentdir = os.path.dirname(currentdir)
sys.path.insert(0, parentdir)
from Global.fit_variables import fit_variables
from Global.selection_variables import *
from Global.cuts import *
from Global.paths import root_storage
import re


def Single_Variable_Plot(
        file_name,
        tree_name,
        variable,
        alias="",
        cut=no_cut,
        p_address="Plots/",
        h_address="Hists/",
        MCTrue=False):
    """
    This function creates and saves 1D hist.
    """
    gROOT.SetBatch()
    gStyle.SetOptStat(0)
    f = TFile(file_name)
    tree = f.Get(tree_name)
    nBins = 100
    mode_name = file_name.split("/")[-1].split(".")[0] + "_" + tree_name
    if MCTrue:
        varname = variable['tuple_name'].replace("B_", "B_MCT_")
    else:
        varname = variable['tuple_name']
    if alias == "":
        obj_name = mode_name + "_" + varname + cut["cut_name"]
        plot_name = varname
    else:
        obj_name = mode_name + "_" + alias + cut["cut_name"]
        plot_name = alias
    if not os.path.exists(p_address + mode_name):
        os.system("mkdir -p " + p_address + mode_name)
    if not os.path.exists(h_address):
        os.system("mkdir -p " + h_address)
    h = TH1F("histo", ";" + variable['xaxis'] + ";", nBins, variable['lLim'], variable['uLim'])
    tree.Project("histo", varname, cut["cut"])
    c = TCanvas("c", "c", 900, 900)
    h.Draw()
    c.SaveAs(p_address + mode_name + "/" + plot_name + ".pdf")
    mf = TFile.Open(h_address + obj_name + ".root", "RECREATE")
    h.Write()
    mf.Close()
    return h_address + obj_name + ".root"


def Variable_Plot_2D(
        file_name,
        tree_name,
        variablex,
        variabley,
        alias="",
        cut=no_cut,
        p_address="Plots/",
        h_address="Hists/",
        MCTrue=False):
    """
    This function creates and saves 2D hist.
    """
    gROOT.SetBatch()
    gStyle.SetOptStat(0)
    f = TFile(file_name)
    tree = f.Get(tree_name)
    mode_name = file_name.split("/")[-1].split(".")[0] + "_" + tree_name
    if MCTrue:
        varnamex = variablex['tuple_name'].replace("B_", "B_MCT_")
        varnamey = variabley['tuple_name'].replace("B_", "B_MCT_")
    else:
        varnamex = variablex['tuple_name']
        varnamey = variabley['tuple_name']
    if alias == "":
        obj_name = mode_name + "_" + varnamex + "_vs_" + varnamey + cut["cut_name"]
        plot_name = varnamex + "_vs_" + varnamey + "_" + cut["cut_name"]
    else:
        obj_name = mode_name + "_" + alias + "_" + cut["cut_name"]
        plot_name = alias
    os.system("mkdir -p " + p_address + mode_name)
    os.system("mkdir -p " + h_address)
    nBins = 100
    h = TH2F(
        "histo",
        ";" +
        variablex['xaxis'] +
        ";" +
        variabley['xaxis'],
        nBins,
        variablex['lLim'],
        variablex['uLim'],
        nBins,
        variabley['lLim'],
        variabley['uLim'])
    tree.Project("histo", varnamey + ":" + varnamex, cut["cut"])
    c = TCanvas("c", "c", 900, 900)
    h.Draw("COLZ")
    c.SaveAs(p_address + mode_name + "/" + plot_name + ".pdf")
    mf = TFile.Open(h_address + obj_name + ".root", "RECREATE")
    h.Write()
    mf.Close()
    return h_address + obj_name + ".root"


def binom_err(k, N):
    """
    This function returns ratio and its uncertainty calculated
    with assumption of binomial distribution.
    """
    if N == 0:
        if k == 0:
            ratio = 1
            err = 0.01
        else:
            ratio = 1.1
            err = 0.1
    else:
        if k > N:
            ratio = 2.
            err = 0.1
        else:
            ratio = k / N

            err = (1 / N) * (k * (1 - k / N))**0.5
    return[ratio, err]


def eff_err(k, N):
    """
    This function returns ratio and its uncertainty calculated
    with assumption of binomial distribution.
    It treats special cases differently from binom_err(k, N).
    """
    if N == 0:
        if k == 0:
            ratio = 0
            err = 0
        else:
            ratio = -0.1
            err = 0.2
    else:
        if k > N:
            ratio = -0.1
            err = 0.2
        else:
            ratio = k / N
            err = (1 / N) * (k * (1 - k / N))**0.5
    return[ratio, err]


def indep_err(a, b):
    """
    This function returns ratio and its uncertainty for
    two independent variables.
    """
    if b == 0:
        if a == 0:
            ratio = 0
            err = 0
        else:
            ratio = -1
            err = 0
    elif a == 0:
        ratio = 0
        err = 0
    else:
        ratio = a / b
        err = ratio * (1 / a + 1 / b)**0.5
    return[ratio, err]


def hist_algebra(hists, extra):
    """
    This function defines operations with histograms,
    which can be preformed when several histograms
    are plot together. Now defined:
        a//b - for each bin of hists a and b, ther ratio
        is calculated with binomial error.

        a/b - for each bin of hists a and b, ther ratio
        is calculated with assumption of their independence

        a%b - for each bin of a and b, a/(a+b) ratio is claculated
    """
    if re.match("[0-9]//[0-9]", extra['expression']):
        h1 = int(extra['expression'].split("//")[0])
        h2 = int(extra['expression'].split("//")[1])
        nBins = hists[h1]["hist"].GetNbinsX()
        h = TH1F(
            "histo",
            ";;" + extra['yaxis'],
            nBins,
            hists[h1]["hist"].GetBinLowEdge(1),
            hists[h1]["hist"].GetBinLowEdge(
                nBins + 1))
        for b in range(1, nBins + 1):
            k = hists[h1]["hist"].GetBinContent(b)
            N = hists[h2]["hist"].GetBinContent(b)
            h.SetBinContent(b, binom_err(k, N)[0])
            h.SetBinError(b, binom_err(k, N)[1])
        return h
    if re.match("[0-9]%[0-9]", extra['expression']):
        h1 = int(extra['expression'].split("%")[0])
        h2 = int(extra['expression'].split("%")[1])
        nBins = hists[h1]["hist"].GetNbinsX()
        h = TH1F(
            "histo",
            ";;" + extra['yaxis'],
            nBins,
            hists[h1]["hist"].GetBinLowEdge(1),
            hists[h1]["hist"].GetBinLowEdge(
                nBins + 1))
        for b in range(1, nBins + 1):
            k = hists[h1]["hist"].GetBinContent(b)
            N = k + hists[h2]["hist"].GetBinContent(b)
            h.SetBinContent(b, binom_err(k, N)[0])
            h.SetBinError(b, binom_err(k, N)[1])
        return h
    if re.match("[0-9]/[0-9]", extra['expression']):
        h1 = int(extra['expression'].split("/")[0])
        h2 = int(extra['expression'].split("/")[1])
        nBins = hists[h1]["hist"].GetNbinsX()
        h = TH1F(
            "histo",
            ";;" + extra['yaxis'],
            nBins,
            hists[h1]["hist"].GetBinLowEdge(1),
            hists[h1]["hist"].GetBinLowEdge(
                nBins + 1))
        for b in range(1, nBins + 1):
            k = hists[h1]["hist"].GetBinContent(b)
            N = hists[h2]["hist"].GetBinContent(b)
            h.SetBinContent(b, indep_err(k, N)[0])
            h.SetBinError(b, indep_err(k, N)[1])
        return h


def Plot_Hists_Together(
        hists,
        normalized=True,
        name="",
        leg_pos=[
            0.6,
            0.7,
            0.9,
            0.9],
        add_legend=True,
        address="Plots/Merged",
        logY=False,
        extra="",
        leg_columns=4):
    """
     Hists should be defined a list of following dictionaries:
     hist = {hist_addr:<address of .root file with histo>,
            name:<String to be shown in TLegend>
            }
    """
    gROOT.SetBatch()

    colors = [2, 4, 8, 46, 6, 40, 3, 7]
    mstyle = [20, 21, 22, 23, 29, 33, 34]

    empty_hists = []
    f = {}
    for h, hist in enumerate(hists):
        f[h] = TFile(hists[h]["hist_addr"])
        hists[h]["hist"] = f[h].Get("histo")
        if hists[h]["hist"].Integral() == 0:
            print("Dropping " + hists[h]["name"] + " hist since it's empty")
            empty_hists.append(hists[h])
            continue
        if normalized:
            integral = hists[h]["hist"].Integral()
            for bin in range(1, hists[h]["hist"].GetNbinsX() + 1):
                hists[h]["hist"].SetBinContent(bin, hists[h]["hist"].GetBinContent(bin) / integral)
                hists[h]["hist"].SetBinError(bin, hists[h]["hist"].GetBinError(bin) / integral)
        hists[h]["hist"].SetLineColor(colors[h % len(colors)])
        hists[h]["hist"].SetLineStyle(h % 10)
        hists[h]["hist"].SetMarkerColor(colors[h % len(colors)])
        hists[h]["hist"].SetLineWidth(3)
        hists[h]["hist"].SetMarkerStyle(mstyle[h % len(mstyle)])

    if extra != "":
        extra_hist = hist_algebra(hists, extra)

    for h in empty_hists:
        hists.remove(h)
    if len(hists) == 0:
        return

    leg = ROOT.TLegend(leg_pos[0], leg_pos[1], leg_pos[2], leg_pos[3])
    leg.SetNColumns(leg_columns)
    max_val = hists[0]["hist"].GetBinContent(hists[0]["hist"].GetMaximumBin())
    min_val = hists[0]["hist"].GetBinContent(hists[0]["hist"].GetMinimumBin())
    for h, hist in enumerate(hists):
        if hists[h]["hist"].GetBinContent(hists[h]["hist"].GetMaximumBin()) > max_val:
            max_val = hists[h]["hist"].GetBinContent(hists[h]["hist"].GetMaximumBin())
        if hists[h]["hist"].GetBinContent(hists[h]["hist"].GetMinimumBin()) < min_val:
            min_val = hists[h]["hist"].GetBinContent(hists[h]["hist"].GetMinimumBin())

    hists[0]["hist"].SetMaximum(max_val * 1.1)
    if normalized:
        hists[0]["hist"].SetMinimum(0)
    else:
        hists[0]["hist"].SetMinimum(min_val)
    gStyle.SetOptStat(0)
    c = TCanvas("c", "c", 900, 900)
    c.SetTopMargin(0.05)
    c.SetBottomMargin(0.2)

    if logY:
        gPad.SetLogy()
    hists[0]["hist"].Draw("E1")
    leg.AddEntry(hists[0]["hist"], hists[0]["name"], "lp")
    for i in range(1, len(hists)):
        gStyle.SetOptStat(0)
        hists[i]["hist"].Draw("SAMEE1")
        leg.AddEntry(hists[i]["hist"], hists[i]["name"], "lp")

    if extra != "":
        rightmax = 1.1 * extra_hist.GetMaximum()
        if rightmax != 0:
            scale = gPad.GetUymax() / rightmax
            extra_hist.SetLineColor(2)
            extra_hist.Scale(scale)
            extra_hist.Draw("samee1")
            axis = TGaxis(gPad.GetUxmax(), gPad.GetUymin(), gPad.GetUxmax(), gPad.GetUymax(), 0, rightmax, 510, "+L")
            axis.SetLineColor(2)
            axis.SetTextColor(2)
            axis.Draw()
            leg.AddEntry(extra_hist, extra["yaxis"], "lp")
    if add_legend:
        leg.Draw()

    if not os.path.exists(address):
        os.system("mkdir -p " + address)
    c.SaveAs(address + "/Merged_" + name + ".pdf")
    return c


def Compare(samples, variables, description, extra="", Normalized=False):
    """
    For each variable of the given set, this function plots together
    corresponding distribution for several samples.
    """
    for varset in variables:
        for var in varset:
            hists = []
            logY = False
            try:
                logY = varset[var]["logY"]
            except BaseException:
                pass
            for s in samples:
                hist_addr = Single_Variable_Plot(s['file_name'], s['tree_name'], varset[var], var, s["cut"], MCTrue=s['MCTrue'])
                hists.append({"hist_addr": hist_addr, "name": s['alias']})
            Plot_Hists_Together(
                hists,
                Normalized,
                var,
                leg_pos=[
                    0.0,
                    0.0,
                    1.0,
                    0.1],
                logY=logY,
                address="Plots/" +
                description,
                extra=extra,
                leg_columns=3)
    return


def Var_Resolution(tree, variable, cut=no_cut, nBins=100):
    """
    This function returns resolution of given variable
    """
    h = TH1F("h", "Resolution of " + variable['xaxis'], nBins, variable['lLim'], variable['uLim'])
    mct_tuple_name = variable['tuple_name'].replace("B_", "B_MCT_")
    tree.Project("h", variable['tuple_name'], cut["cut"])
    return [h.GetStdDev(), h.GetStdDevError()]


def Resolution_Plots(file_name, tree_name, variable, alias="", cut=no_cut, p_address="Plots/", h_address="Hists/"):
    """
    This function does resolution study of fit variable.
    """
    gROOT.SetBatch()
    gStyle.SetOptStat(0)
    f = TFile(file_name)
    tree = f.Get(tree_name)
    nBins = 20
    mode_name = file_name.split("/")[-1].split(".")[0] + "_" + tree_name
    if alias == "":
        obj_name = mode_name + "_" + variable['tuple_name'] + cut["cut_name"]
        plot_name1 = variable['tuple_name'] + "_Gen_vs_Rec"
        plot_name2 = variable['tuple_name'] + "_Gen_vs_Bias"
        plot_name3 = variable['tuple_name'] + "_Rec_vs_Bias"
        scan_name = variable['tuple_name'] + "_res_scan"
    else:
        obj_name = mode_name + "_" + alias + cut["cut_name"]
        plot_name1 = alias + "_Gen_vs_Rec"
        plot_name2 = alias + "_Gen_vs_Bias"
        plot_name3 = alias + "_Rec_vs_Bias"
        scan_name = alias + "_res_scan"
    if not os.path.exists(p_address + mode_name + "/" + cut["cut_name"]):
        os.system("mkdir -p " + p_address + mode_name + "/" + cut["cut_name"])
    if not os.path.exists(h_address):
        os.system("mkdir -p " + h_address)

    mct_tuple_name = variable['tuple_name'].replace("B_", "B_MCT_")
    llim = variable['lLim']
    ulim = variable['uLim']
    try:
        mcllim = variable['MClLim']
        mculim = variable['MCuLim']
    except BaseException:
        mcllim = variable['lLim']
        mculim = variable['uLim']
    res_scale = 1
    if variable['tuple_name'] == "B_Mbc_corr":
        res_scale = 0.01

    h1 = TH2F(
        "GenVsRec",
        ";Generated " +
        variable['xaxis'] +
        ";Reconstructed " +
        variable['xaxis'],
        nBins,
        mcllim,
        mculim,
        nBins,
        llim,
        ulim)
    h2 = TH2F("GenVsBias", ";Generated " +
              variable['xaxis'] +
              ";Bias " +
              variable['xaxis'], nBins, mcllim, mculim, nBins, -
              res_scale *
              ulim, res_scale *
              ulim)
    h3 = TH2F("RecVsBias", ";Reconstructed " +
              variable['xaxis'] +
              ";Bias " +
              variable['xaxis'], nBins, llim, ulim, nBins, -
              res_scale *
              ulim, res_scale *
              ulim)
    h1.GetYaxis().SetTitleOffset(2)
    h2.GetYaxis().SetTitleOffset(2)
    h3.GetYaxis().SetTitleOffset(2)
    res2 = TH1F("res2", "Resolution of " + variable['xaxis'], nBins, mcllim, mculim)
    res3 = TH1F("res3", "Resolution of " + variable['xaxis'], nBins, llim, ulim)
    res2.SetLineColor(2)
    res2.SetLineWidth(3)
    res3.SetLineColor(2)
    res3.SetLineWidth(3)

    tree.Project("GenVsRec", variable['tuple_name'] + ":" + mct_tuple_name, cut["cut"])
    tree.Project("GenVsBias", "(" + variable['tuple_name'] + '-' + mct_tuple_name + ")" + ":" + mct_tuple_name, cut["cut"])
    tree.Project("RecVsBias", "(" + variable['tuple_name'] + '-' + mct_tuple_name + ")" + ":" + variable['tuple_name'], cut["cut"])

    hists = []
    for var in fit_variables:
        res = TH1F("histo", ";Generated " + variable['xaxis'] + "; #sigma (Arb.units)", nBins, mcllim, mculim)
        res_2D = TH2F(var, var, nBins, mcllim, mculim, 2 * nBins, -ulim, ulim)
        var_mct_tuple_name = fit_variables[var]['tuple_name'].replace("B_", "B_MCT_")
        tree.Project(
            var,
            "(" +
            fit_variables[var]['tuple_name'] +
            '-' +
            var_mct_tuple_name +
            ")" +
            ":" +
            mct_tuple_name,
            cut["cut"])
        for bin in range(1, nBins + 1):
            slice_var = res_2D.ProjectionY("h2 bin " + str(bin), bin - 1, bin)
            if slice_var.GetStdDev() > 0.2:
                res.SetBinContent(bin, 0.1)
                res.SetBinError(bin, 0.1)
            else:
                res.SetBinContent(bin, slice_var.GetStdDev())
                res.SetBinError(bin, slice_var.GetStdDevError())

        obj_name = variable['tuple_name'] + "_res_scan_of_" + var
        hist_addr = h_address + obj_name + ".root"
        mf = TFile.Open(hist_addr, "RECREATE")
        res.Write()
        mf.Close()
        hists.append({"hist_addr": hist_addr, "name": fit_variables[var]['xaxis'] + " res."})
    Plot_Hists_Together(hists, True, scan_name, leg_pos=[0.0, 0.0, 1.0, 0.1], logY=False, address=p_address + mode_name)

    for bin in range(1, nBins + 1):
        slice2 = h2.ProjectionY("h2 bin " + str(bin), bin - 1, bin)
        slice3 = h3.ProjectionY("h3 bin " + str(bin), bin - 1, bin)
        res2.SetBinContent(bin, slice2.GetStdDev())
        res2.SetBinError(bin, slice2.GetStdDevError())
        res3.SetBinContent(bin, slice3.GetStdDev())
        res3.SetBinError(bin, slice3.GetStdDevError())

    c = TCanvas("c", "c", 900, 900)
    c.SetLeftMargin(0.15)
    c.SetRightMargin(0.15)
    h1.Draw("COLZ")
    c.SaveAs(p_address + mode_name + "/" + plot_name1 + ".pdf")

    leg = ROOT.TLegend(0.15, 0.9, 0.8, 1.)
    leg.SetNColumns(2)
    c.SetRightMargin(0.2)
    h2.Draw("COLZ")
    c.Update()
    palette = h2.GetListOfFunctions().FindObject("palette")
    palette.SetX1NDC(0.88)
    palette.SetX2NDC(0.93)
    palette.SetY1NDC(0.1)
    palette.SetY2NDC(0.9)
    palette.SetLabelSize(0.03)
    c.Update()
    prof2 = h2.ProfileX()
    prof2.Draw("samee1")
    leg.AddEntry(prof2, "Mean. bias", "lp")
    rightmax = 1.1 * res2.GetMaximum()
    scale = gPad.GetUymax() / rightmax
    res2.Scale(scale)
    res2.Draw("samee1")
    leg.AddEntry(res2, "Resolution", "lp")
    axis = TGaxis(gPad.GetUxmax(), gPad.GetUymin(), gPad.GetUxmax(), gPad.GetUymax(), 0, rightmax, 510, "+L")
    axis.SetLineColor(2)
    axis.SetTextColor(2)
    axis.SetLabelColor(2)
    axis.SetLabelSize(0.03)
    axis.Draw()
    leg.Draw()
    c.SaveAs(p_address + mode_name + "/" + plot_name2 + ".pdf")

    leg = ROOT.TLegend(0.15, 0.9, 0.8, 1.)
    leg.SetNColumns(2)
    h3.Draw("COLZ")
    c.Update()
    palette = h3.GetListOfFunctions().FindObject("palette")
    palette.SetX1NDC(0.88)
    palette.SetX2NDC(0.93)
    palette.SetY1NDC(0.1)
    palette.SetY2NDC(0.9)
    palette.SetLabelSize(0.03)
    c.Update()
    prof3 = h3.ProfileX()
    prof3.Draw("samee1")
    leg.AddEntry(prof3, "Mean. bias", "lp")
    rightmax = 1.1 * res3.GetMaximum()
    scale = gPad.GetUymax() / rightmax
    res3.Scale(scale)
    res3.Draw("samee1")
    leg.AddEntry(res3, "Resolution", "lp")
    axis = TGaxis(gPad.GetUxmax(), gPad.GetUymin(), gPad.GetUxmax(), gPad.GetUymax(), 0, rightmax, 510, "+L")
    axis.SetLineColor(2)
    axis.SetTextColor(2)
    axis.SetLabelColor(2)
    axis.SetLabelSize(0.03)
    axis.Draw()
    leg.Draw()
    c.SaveAs(p_address + mode_name + "/" + plot_name3 + ".pdf")

    mf = TFile.Open(h_address + obj_name + ".root", "RECREATE")
    h1.Write()
    h2.Write()
    h3.Write()
    mf.Close()
    return h_address + obj_name + ".root"
