#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
# Given reconstructed data from runSimReco_Belle3_validation
# makes efficiency  plots
#
# ./plot_efficiency.py -f1 belle3_analysis_dimuon_all_charged.root
#
"""

import ROOT as r

# Plot reconstructed & generated var with its efficiency, residus and pull


def plot_Efficiency(rootfile, listvariable, out_file, save_individual_pdf=False):

    # Get the root files
    f1 = r.TFile.Open(rootfile)

    t1 = f1.variables

    c = r.TCanvas('c', 'Results efficiency', 900, 900)
    c.Divide(2, 2)

    for i in range(len(listvariable)):

        # Get all the parameters
        var = listvariable[i][0]
        xmin = listvariable[i][1][0]
        xmax = listvariable[i][1][1]
        nbins = listvariable[i][2]

        # Define mcVar & varErr
        if var in ["theta", "phi", "cosTheta"]:
            mcVar = "mc" + var[0].upper() + var[1:]
        else:
            mcVar = "mc" + var.upper()

        if var in ["E", "x", "y", "z"]:
            varErr = var + "_uncertainty"
        else:
            varErr = var + "Err"

        # cut = "mcPrimary==1 && mcInitial==0 && mcVirtual==0 && isSignal==1"
        cut = "mcPrimary==1 && mcInitial==0 && mcVirtual==0"
        # cut = ""
        cutMC = ""

        # Define histo
        thRec = r.TH1D('thRec' + var, var + ' reconstructed', nbins, xmin, xmax)
        thGen = r.TH1D('thGen' + var, var + ' generated', nbins, xmin, xmax)
        thEff = r.TH1D('thEff' + var, var + ' efficiency', nbins, xmin, xmax)

        thRes = r.TH1D('thRes' + var, var + ' residus', nbins, -0.02, 0.02)
        thPul = r.TH1D('thPul' + var, var + ' pull', nbins, -6, 6)

        # Draw histo
        t1.Draw(var + '>>thRec' + var, cut, "goff")
        t1.Draw(mcVar + '>>thGen' + var, cutMC, "goff")
        thEff.Divide(thRec, thGen, 1, 1, "b")

        t1.Draw('(' + var + '-' + mcVar + ')>>thRes' + var, cut, "goff")
        t1.Draw('(' + var + '-' + mcVar + ')/(' + varErr + ')>>thPul' + var, cut, "goff")

        gauss = r.TF1("gauss" + var, "gaus", -6, 6)
        thPul.Fit(gauss, 'RNQ')

        # leg = r.TLegend( .58, .75, .9, .9, "")
        leg = r.TLegend(.58, .26, .9, .4, "")
        leg.SetTextSize(0.04)
        leg.SetFillColor(0)
        leg.SetFillStyle(1001)
        leg.AddEntry(thRec, "Reconstructed", "l")
        leg.AddEntry(thGen, "Generated", "l")
        leg.SetBorderSize(1)
        leg.SetFillStyle(1)

        thGen.SetLineColor(2)
        thGen.SetStats(False)
        thGen.GetXaxis().SetTitle(var)
        thGen.GetYaxis().SetTitle("Events")

        thRec.SetStats(False)

        thEff.GetXaxis().SetTitle(var)
        thEff.GetYaxis().SetTitle("efficiency")
        thEff.SetMinimum(0.)
        # thEff.SetMaximum(1.)
        thEff.SetLineColor(1)
        thEff.SetStats(False)

        thRec.SetLineColor(1)
        thRec.SetStats(False)

        thRes.GetXaxis().SetTitle('delta' + var)
        thRes.GetYaxis().SetTitle("Events")

        thPul.GetXaxis().SetTitle("Pull")
        thPul.GetYaxis().SetTitle("Events")

        c.cd(1)
        r.gPad.SetLeftMargin(0.15)
        thGen.Draw()
        thRec.Draw("SAME")
        leg.Draw()
        c.cd(2)
        r.gPad.SetLeftMargin(0.15)
        thEff.Draw()
        c.cd(3)
        r.gPad.SetLeftMargin(0.15)
        thRes.Draw()
        c.cd(4)
        r.gPad.SetLeftMargin(0.15)
        thPul.Draw()
        gauss.Draw("SAME")

        if save_individual_pdf:
            c.SaveAs("pdf/results" + var + ".pdf")
        c.Print(out_file)


if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('-f1', '--file1', required=True, type=str, help='Path to the reconstructed data')
    args = parser.parse_args()

    r.TH1.SetDefaultSumw2()
    r.gStyle.SetOptStat(2222)
    r.gStyle.SetOptFit(1111)
    r.gStyle.SetStatW(0.3)
    r.gStyle.SetStatH(0.1)

    cpdf = r.TCanvas('cpdf', 'Results', 900, 900)
    out_file = 'efficiencies.pdf'
    cpdf.Print(out_file + '[')

    # Rootfiles used
    rootfiles = args.file1

    # Defining list for variables to get efficiency
    # name = ['leaf on the tree',[xmin,xmax],number of bins]
    p = ['p', [0, 7], 20]
    pt = ['pt', [0, 7], 20]
    theta = ['theta', [0.29, 2.62], 10]
    phi = ['phi', [-3.2, 3.2], 10]

    # Efficiency(rootfile, [variablelist1,...], outfile)
    plot_Efficiency(rootfiles, [p, pt, theta, phi], out_file)

    cpdf.Print(out_file + ']')
    cpdf.Close()
    input()
