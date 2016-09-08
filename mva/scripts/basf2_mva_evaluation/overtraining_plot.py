#!/usr/bin/env python
# -*- coding: utf-8 -*-

import ROOT

import array
import math
import basf2_mva_evaluation.utility as utility


def _from_hists(signalHist, bckgrdHist):
    pass


def _from_ntuple(ntuple, probability, truth, nbins=100):
    bckgrdHist = ROOT.TH1D('ROCbackground', '', nbins, 0.0, 1.0)
    signalHist = ROOT.TH1D('ROCsignal', '', nbins, 0.0, 1.0)
    signalHist.SetTitle(";Classifier Output;Entries (norm.)")
    bckgrdHist.Sumw2()
    signalHist.Sumw2()
    ntuple.Project('ROCbackground', probability, '!' + truth)
    ntuple.Project('ROCsignal', probability, truth)
    _from_hists(signalHist, bckgrdHist)

    # normalize to 1
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
    legend_left = ROOT.TLegend(0.15, 0.77, 0.45, 0.9)
    legend_left.SetBorderSize(0)
    legend_left.SetFillColor(0)
    legend_left.SetFillStyle(0)
    legend_left.SetTextFont(42)
    legend_left.SetTextSize(0.03)

    legend_right = ROOT.TLegend(0.47, 0.77, 0.77, 0.9)
    legend_right.SetBorderSize(0)
    legend_right.SetFillColor(0)
    legend_right.SetFillStyle(0)
    legend_right.SetTextFont(42)
    legend_right.SetTextSize(0.03)

    for i, (probability, truth, label) in enumerate(zip(probabilities, truths, labels)):
        signalHist_train, bckgrdHist_train = _from_ntuple(train_ntuple, probability, truth)
        drawSignal(signalHist_train, i)
        drawBckgrd(bckgrdHist_train, i)
        legend_left.SetHeader(label)
        legend_left.AddEntry(signalHist_train, "Signal (training set)", "f")
        legend_left.AddEntry(bckgrdHist_train, "Background (training set)", "f")

        signalHist_test, bckgrdHist_test = _from_ntuple(test_ntuple, probability, truth)
        drawSignal(signalHist_test, i + 1)
        drawBckgrd(bckgrdHist_test, i + 1)
        # Kolmogorov-Smirnov test
        kv_signal = signalHist_train.KolmogorovTest(signalHist_test)
        kv_bckgrd = bckgrdHist_train.KolmogorovTest(bckgrdHist_test)
        legend_right.SetHeader("KS test   Signal (Background):   " + str(kv_signal) + " (" + str(kv_bckgrd) + ")")
        legend_right.AddEntry(signalHist_test, "Signal (test set)", "f")
        legend_right.AddEntry(bckgrdHist_test, "Background (test set)", "f")

    legend_left.Draw("same")
    legend_right.Draw("same")

    canvas.SaveAs(outputfilename)
