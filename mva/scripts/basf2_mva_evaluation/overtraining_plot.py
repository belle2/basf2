#!/usr/bin/env python
# -*- coding: utf-8 -*-

import ROOT

import array
import math
import basf2_mva_evaluation.utility as utility


def _from_hists(signalHist, bckgrdHist):
    pass


def _from_ntuple(ntuple, probability, truth, nbins=100):
    bckgrdHist = ROOT.TH1D('ROCbackground', 'background', nbins, 0.0, 1.0)
    signalHist = ROOT.TH1D('ROCsignal', 'signal', nbins, 0.0, 1.0)
    bckgrdHist.Sumw2()
    signalHist.Sumw2()
    ntuple.Project('ROCbackground', probability, '!' + truth)
    ntuple.Project('ROCsignal', probability, truth)
    _from_hists(signalHist, bckgrdHist)
    bckgrdHist.Scale(1.0 / bckgrdHist.Integral(), "width")
    signalHist.Scale(1.0 / signalHist.Integral(), "width")
    signalHist.ROOT_OBJECT_OWNERSHIP_WORKAROUND = signalHist
    bckgrdHist.ROOT_OBJECT_OWNERSHIP_WORKAROUND = bckgrdHist
    return signalHist, bckgrdHist


def drawSignal(signalHist, i):
    if i == 0:
        signalHist.SetFillColor(ROOT.TColor.GetColor("#7d99d1"))
        signalHist.SetFillStyle(1001)
        signalHist.SetLineColor(ROOT.TColor.GetColor("#0000ee"))
        signalHist.SetLineWidth(2)
        signalHist.Draw("samehis")
    elif i == 1:
        signalHist.SetMarkerColor(ROOT.TColor.GetColor("#0000ee"))
        signalHist.SetMarkerSize(0.7)
        signalHist.SetMarkerStyle(20)
        signalHist.SetLineWidth(1)
        signalHist.SetLineColor(ROOT.TColor.GetColor("#0000ee"))
        signalHist.Draw("e1same")
    else:
        print("Distribution plot of more than two classifiers is not supported")


def drawBckgrd(bckgrdHist, i):
    if i == 0:
        bckgrdHist.SetFillColor(ROOT.TColor.GetColor("#ff0000"))
        bckgrdHist.SetFillStyle(3554)
        bckgrdHist.SetLineColor(ROOT.TColor.GetColor("#ff0000"))
        bckgrdHist.SetLineWidth(2)
        bckgrdHist.Draw("samehist")
    elif i == 1:
        bckgrdHist.SetMarkerColor(ROOT.TColor.GetColor("#ff0000"))
        bckgrdHist.SetMarkerSize(0.7)
        bckgrdHist.SetMarkerStyle(20)
        bckgrdHist.SetLineWidth(1)
        bckgrdHist.SetLineColor(ROOT.TColor.GetColor("#ff0000"))
        bckgrdHist.Draw("e1same")
    else:
        print("Distribution plot of more than two classifiers is not supported")


def from_file(train_rootfile, test_rootfile, probabilities, truths, labels, outputfilename, nbins=100):
    train_ntuple = train_rootfile.Get("variables")
    test_ntuple = test_rootfile.Get("variables")
    canvas = ROOT.TCanvas("canvas", "Overtraining plot", 1600, 1200)
    canvas.cd()

    ROOT.gStyle.SetOptStat(ROOT.kFALSE)
    legend = ROOT.TLegend(0.1, 0.7, 0.48, 0.9)

    for i, (probability, truth, label) in enumerate(zip(probabilities, truths, labels)):
        signalHist, bckgrdHist = _from_ntuple(train_ntuple, probability, truth)
        drawSignal(signalHist, i)
        drawBckgrd(bckgrdHist, i)
        legend.AddEntry(signalHist, "(Train) Signal " + label, "f")
        legend.AddEntry(bckgrdHist, "(Train) Background " + label, "f")
        signalHist, bckgrdHist = _from_ntuple(test_ntuple, probability, truth)
        drawSignal(signalHist, i+1)
        drawBckgrd(bckgrdHist, i+1)
        legend.AddEntry(signalHist, "(Test) Signal " + label, "f")
        legend.AddEntry(bckgrdHist, "(Test) Background " + label, "f")
    legend.Draw("same")

    canvas.SaveAs(outputfilename)
