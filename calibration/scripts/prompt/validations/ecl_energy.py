##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
ECL Energy Validation
"""

from prompt import ValidationSettings
import sys

##############################
# REQUIRED VARIABLE #
##############################
# Will be used to construct the validation in the automated system, as well as set up the submission web forms.

#: Tells the automated system some details of this script
settings = ValidationSettings(name='ECL_Energy',
                              description=__doc__,
                              download_files=['stdout'],
                              expert_config=None)


def run_validation(job_path, input_data_path, expert_config):
    # job_path will be replaced with path/to/calibration_results
    # input_data_path will be replaced with path/to/data_path used for calibration, e.g. /group/belle2/dataprod/Data/PromptSkim/

    # Verify that output from airflow is OK
    from ROOT import TH1F, TCanvas, TFile, TGraph, TLine, kRed, gStyle
    import numpy as np
    import os
    from array import array

    if not os.path.exists('plots'):
        os.makedirs('plots')

    # Add underflow, overflow and fit result
    gStyle.SetOptStat(1111111)
    gStyle.SetOptFit(1111)

    # ------------------------------------------------------------------------
    # Validation histograms
    ggRatio = TH1F("ggRatio", "Ratio of output to input calib, gamma gammaRatio", 200, 0.9, 1.1)

    eeRatio = TH1F("eeRatio", "Ratio of output to input calib, Bhabha5x5Ratio", 400, 0.9, 1.1)
    eeggRatio = TH1F("eeggRatio", "Ratio of ee/gg calibrations, thetaID [14,57]Ratio", 200, 0.95, 1.05)

    mumuggRatio = TH1F("mumuggRatio", "Ratio of muon pair/gg calibrations, thetaID [14,57]Ratio", 200, 0.93, 1.03)

    checkMerge = TH1F("checkMerge", "Ratio between final and gg calib valuesRatio", 200, 0., 2.)
    finalRatio = TH1F("finalRatio", "Ratio of new/existing energy calibrationsratio", 200, 0.95, 1.05)

    myC = TCanvas("myC")

    # ------------------------------------------------------------------------
    # Gamma Gamma
    print("\n---------------------------------------- \nGamma gamma comparison: \n\n")
    gg = TFile(f"{job_path}/ecl_gamma_gamma/0/algorithm_output/eclGammaGammaE_algorithm.root")

    # Entries / luminosity
    EnVsCrysID = gg.Get("EnVsCrysID")
    ggEntries = EnVsCrysID.GetEntries()
    crossSection = 3990000.  # fb
    lumi = ggEntries / crossSection
    nomLum = 2.36  # fb-1
    estUnc = 0.5 * np.sqrt(nomLum / lumi)

    print(f"{ggEntries:f} entries in gg EnVsCrysID lumi = {lumi:.2f} fb-1 est uncerta = {estUnc:.2f} \n")

    # Summarize fit status
    hStatusgg = gg.Get("hStatus")
    success = 100. * (hStatusgg.GetBinContent(22) + hStatusgg.GetBinContent(14)) / 8736.
    print(f"\nSummary of Gamma Gamma fit status. {success:.1f} good fits:\n")
    print(f"16 good fit:   {hStatusgg.GetBinContent(22):.4f} \n")
    print(f" 8 iterations: {hStatusgg.GetBinContent(14):.4f} \n")
    print(f" 4 at limit:   {hStatusgg.GetBinContent(10):.4f} \n")
    print(f" 3 poor fit:   {hStatusgg.GetBinContent(9):.4f} \n")
    print(f"-1 low stats:  {hStatusgg.GetBinContent(5):.4f} \n")

    # Compare output to input (gamma gamma) calibrations
    StatusVsCrysIDgg = gg.Get("StatusVsCrysID")
    AverageInitCalibgg = gg.Get("AverageInitCalib")
    CalibVsCrysIDgg = gg.Get("CalibVsCrysID")

    print("\nCompare gamma gamma output calibration to input for large changes:\n")
    bigChange = 0
    minBarrelRatio = 99.
    minCellID = -1
    maxBarrelRatio = 0.
    maxCellID = -1
    for cellID in range(1, 8736 + 1):
        status = StatusVsCrysIDgg.GetBinContent(cellID)
        if status > 0:
            inputCalib = AverageInitCalibgg.GetBinContent(cellID)
            outputCalib = CalibVsCrysIDgg.GetBinContent(cellID)
            ratio = outputCalib / inputCalib
            ggRatio.Fill(ratio)
            if(ratio < 0.95 or ratio > 1.05):
                bigChange += 1
                print(f"{bigChange:.2f} cellID {cellID:.4f} {ratio:.3f} {status}\n")

            # Look for large deviations in the barrel excluding first and last
            if(cellID >= 1297 and cellID <= 7632):
                if(ratio < minBarrelRatio):
                    minBarrelRatio = ratio
                    minCellID = cellID
                if(ratio > maxBarrelRatio):
                    maxBarrelRatio = ratio
                    maxCellID = cellID

    ggRatio.Fit("gaus")
    myC.Print("plots/ggRatio.pdf")

    print("\nLargest changes in thetaID [14,57]\n")
    print(f"ratio = {minBarrelRatio:.3f} in cellID {minCellID:f} \n")
    print(f"ratio = {maxBarrelRatio:.3f} in cellID {maxCellID:f} \n")

    histMean = ggRatio.GetMean()
    ggfit = ggRatio.GetFunction("gaus")
    fitMean = ggfit.GetParameter(1)
    fitSigma = ggfit.GetParameter(2)
    print(
        "\n\nRatio of New/old Gamma gamma calib consts: mean = %.4f mean of Gaussian fit = %.4f, sigma = %.4f\n" %
        (histMean, fitMean, fitSigma))

    # ------------------------------------------------------------------------
    # Bhabha
    print("\n---------------------------------------- \nBhabha comparison: \n\n")
    ee = TFile(f"{job_path}/ecl_ee5x5/0/algorithm_output/eclee5x5Algorithm.root")

    # Compare output to input calibrations
    AverageInitCalibee = ee.Get("AverageInitCalib")
    CalibVsCrysIDee = ee.Get("CalibVsCrysID")

    print("\nCompare Bhabha output calibration to input:\n")
    bigChange = 0
    eeCalibrated = 0
    for cellID in range(1, 8736 + 1):
        inputCalib = AverageInitCalibee.GetBinContent(cellID)
        outputCalib = CalibVsCrysIDee.GetBinContent(cellID)
        if(outputCalib > 0):
            eeCalibrated += 1
            ratio = outputCalib / inputCalib
            eeRatio.Fill(ratio)
            if(ratio < 0.95 or ratio > 1.05):
                bigChange += 1
                print(f" {int(bigChange):2} cellID {cellID:.4f} {ratio:5.3f}\n")

    print(f"Total calibrated crystals = {eeCalibrated:.4f} = {100.0 * eeCalibrated / 8736.0:.1f} \n\n")
    eeRatio.Fit("gaus")
    myC.Print("plots/eeRatio.pdf")

    histMean = eeRatio.GetMean()
    eefit = eeRatio.GetFunction("gaus")
    fitMean = eefit.GetParameter(1)
    fitSigma = eefit.GetParameter(2)
    print(f"\nBhabha mean ratio = {histMean:.4f} mean of Gaussian fit = {fitMean:.4f}, sigma = {fitSigma:.4f}\n")

    # ------------------------------------------------------------------------
    # Bhabha/gg comparison
    print("\n---------------------------------------- \nBhabha to gamma comparison: \n\n")
    xcellID = array('d', [0] * 8736)
    yratio = array('d', [0] * 8736)
    nPo = 0
    for cellID in range(1, 8736 + 1):
        ggstatus = StatusVsCrysIDgg.GetBinContent(cellID)
        ggCalib = CalibVsCrysIDgg.GetBinContent(cellID)
        eeCalib = CalibVsCrysIDee.GetBinContent(cellID)
        if(ggstatus > 0. and eeCalib > 0.):
            if(cellID >= 1297 and cellID <= 7632):
                eeggRatio.Fill(eeCalib / ggCalib)
                xcellID[nPo] = cellID
                yratio[nPo] = eeCalib / ggCalib
                nPo += 1

    # Fit ratio
    eeggRatio.Fit("gaus")
    myC.Print("plots/eeggRatio.pdf")

    histMean = eeggRatio.GetMean()
    eeggfit = eeggRatio.GetFunction("gaus")
    fitMean = eeggfit.GetParameter(1)
    fitSigma = eeggfit.GetParameter(2)
    print(f"\nBhabha to gg ratio mean = {histMean:.4f} mean of Gaussian fit = {fitMean:.4f}, sigma = {fitSigma:.4f}\n")

    # Graph
    eeggRatiovsCellID = TGraph(nPo, xcellID, yratio)
    eeggRatiovsCellID.SetName("eeggRatiovsCellID")
    eeggRatiovsCellID.SetMarkerStyle(20)
    eeggRatiovsCellID.SetMarkerSize(0.3)
    eeggRatiovsCellID.SetTitle("Bhabha/gg calibration ratiocellIDratio")
    eeggRatiovsCellID.Draw("AP")
    eeggRatiovsCellID.GetXaxis().SetRangeUser(1, 8737)
    eeggRatiovsCellID.GetYaxis().SetRangeUser(0.95, 1.05)
    one = TLine(1, 1, 8737, 1)
    one.SetLineColor(kRed)
    one.SetLineStyle(9)
    one.Draw()
    myC.Print("plots/eeggRatiovsCellID.pdf")

    # ------------------------------------------------------------------------
    # muon pair
    print("\n---------------------------------------- \nMuon pair comparison: \n\n")
    mumu = TFile(f"{job_path}/ecl_mu_mu/0/algorithm_output/eclMuMuEAlgorithm.root")

    # Summarize fit status
    hStatusmumu = mumu.Get("hStatus")
    success = 100. * (hStatusmumu.GetBinContent(22) + hStatusmumu.GetBinContent(14)) / 8736.
    print(r"\nSummary of muon pair fit status. \%.1f good fits:\n", success)
    print(f"16 good fit:   {hStatusmumu.GetBinContent(22):.4f} \n")
    print(f" 8 iterations: {hStatusmumu.GetBinContent(14):.4f} \n")
    print(f" 4 at limit:   {hStatusmumu.GetBinContent(10):.4f} \n")
    print(f" 3 poor fit:   {hStatusmumu.GetBinContent(9):.4f} \n")
    print(f" 2 no peak:    {hStatusmumu.GetBinContent(8):.4f} \n")
    print(f"-1 low stats:  {hStatusmumu.GetBinContent(5):.4f} \n")

    # ------------------------------------------------------------------------
    # Muon pair/gg comparison
    print("\n---------------------------------------- \nMuon pair to gamma comparison: \n\n")
    StatusVsCrysIDmumu = mumu.Get("StatusVsCrysID")
    # AverageInitCalibmumu = mumu.Get("AverageInitCalib")
    CalibVsCrysIDmumu = mumu.Get("CalibVsCrysID")

    nPo = 0
    for cellID in range(1, 8736 + 1):
        ggstatus = StatusVsCrysIDgg.GetBinContent(cellID)
        ggCalib = CalibVsCrysIDgg.GetBinContent(cellID)
        mumustatus = StatusVsCrysIDmumu.GetBinContent(cellID)
        mumuCalib = CalibVsCrysIDmumu.GetBinContent(cellID)
        if(ggstatus > 7.5 and mumustatus > 7.5):
            if(cellID >= 1297 and cellID <= 7632):
                mumuggRatio.Fill(mumuCalib / ggCalib)
                xcellID[nPo] = cellID
                yratio[nPo] = mumuCalib / ggCalib
                nPo += 1

    mumuggRatio.Fit("gaus")
    myC.Print("plots/mumuggRatio.pdf")

    histMean = mumuggRatio.GetMean()
    mumuggfit = mumuggRatio.GetFunction("gaus")
    fitMean = mumuggfit.GetParameter(1)
    fitSigma = mumuggfit.GetParameter(2)
    print(
        f"\nMuon pair to gg ratio mean = {histMean:.4f} mean of Gaussian fit = {fitMean:.4f}, sigma = {fitSigma:.4f}\n")

    # Graph of ratio versus cellID
    mumuggRatiovsCellID = TGraph(nPo, xcellID, yratio)
    mumuggRatiovsCellID.SetName("mumuggRatiovsCellID")
    mumuggRatiovsCellID.SetMarkerStyle(20)
    mumuggRatiovsCellID.SetMarkerSize(0.3)
    mumuggRatiovsCellID.SetTitle("Muon pair/gg calibration ratiocellIDratio")
    mumuggRatiovsCellID.Draw("AP")
    mumuggRatiovsCellID.GetXaxis().SetRangeUser(1, 8737)
    mumuggRatiovsCellID.GetYaxis().SetRangeUser(0.93, 1.03)
    one.Draw()
    myC.Print("plots/mumuggRatiovsCellID.pdf")

    # ------------------------------------------------------------------------
    # Check merge stage
    print("\n---------------------------------------- \nFinal merge comparison: \n\n")
    merge = TFile(f"{job_path}/ecl_merge/0/algorithm_output/ECLCrystalEnergy.root")
    newPayload = merge.Get("newPayload")
    existingPayload = merge.Get("existingPayload")

    # Final calib should be equal to gamma gamma if available unchanged otherwise.
    nPo = 0
    for cellID in range(1, 8736 + 1):
        ggstatus = StatusVsCrysIDgg.GetBinContent(cellID)
        newOldRatio = newPayload.GetBinContent(cellID) / existingPayload.GetBinContent(cellID)
        newggRatio = newPayload.GetBinContent(cellID) / CalibVsCrysIDgg.GetBinContent(cellID)
        if(ggstatus > 0.):
            checkMerge.Fill(newggRatio)
            finalRatio.Fill(newOldRatio)
            xcellID[nPo] = cellID
            yratio[nPo] = newOldRatio
            nPo += 1
        else:
            checkMerge.Fill(newOldRatio)

    checkMerge.Draw()
    myC.Print("plots/checkMerge.pdf")

    finalRatio.Fit("gaus")
    myC.Print("plots/finalRatio.pdf")

    histMean = finalRatio.GetMean()
    finalRatiofit = finalRatio.GetFunction("gaus")
    fitMean = finalRatiofit.GetParameter(1)
    fitSigma = finalRatiofit.GetParameter(2)
    print(f"\nNew to exist ratio mean = {histMean:.4f} mean of Gaussian fit = {fitMean:.4f}, sigma = {fitSigma:.4f}\n")

    # Graph of / old calibration constants
    newOldRatiovsCellID = TGraph(nPo, xcellID, yratio)
    newOldRatiovsCellID.SetName("newOldRatiovsCellID")
    newOldRatiovsCellID.SetMarkerStyle(20)
    newOldRatiovsCellID.SetMarkerSize(0.3)
    newOldRatiovsCellID.SetTitle("New/existing calibration ratiocellIDratio")
    newOldRatiovsCellID.Draw("AP")
    newOldRatiovsCellID.GetXaxis().SetRangeUser(1, 8737)
    newOldRatiovsCellID.GetYaxis().SetRangeUser(0.95, 1.05)
    one.Draw()
    myC.Print("plots/newOldRatiovsCellID.pdf")

    # ------------------------------------------------------------------------
    # Write out histograms
    fout = TFile("validateAirflow.root", "recreate")
    ggRatio.Write()
    eeRatio.Write()
    eeggRatio.Write()
    mumuggRatio.Write()
    checkMerge.Write()
    finalRatio.Write()

    eeggRatiovsCellID.Write()
    mumuggRatiovsCellID.Write()
    newOldRatiovsCellID.Write()
    one.Draw()

    fout.Close()


if __name__ == "__main__":
    run_validation(*sys.argv[1:])
