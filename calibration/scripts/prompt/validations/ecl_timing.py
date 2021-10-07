# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
ECL Timing Validation
"""

from prompt import ValidationSettings
import sys
import ROOT as r
from pathlib import Path
import os

##############################
# REQUIRED VARIABLE #
##############################
# Will be used to construct the validation in the automated system, as well as set up the submission web forms.

#: Tells the automated system some details of this script
settings = ValidationSettings(name='ECL crystal and crate time calibrations and validations',
                              description=__doc__,
                              download_files=['stdout'],
                              expert_config=None)


def run_validation(job_path, input_data_path, requested_iov, expert_config, **kwargs):
    # job_path will be replaced with path/to/calibration_results
    # input_data_path will be replaced with path/to/data_path used for calibration, e.g. /group/belle2/dataprod/Data/PromptSkim/

    # Verify that output from airflow is OK
    from ROOT import TH1F, TCanvas, TFile, TGraph, TLine, kRed, gStyle
    import numpy as np
    import os
    from array import array

    # Set root style so that there are ticks on all four sides of the plots
    r.gStyle.SetPadTickX(1)
    r.gStyle.SetPadTickY(1)

    # Output information about the parameters passed to the function
    print("job_path = ", job_path)
    print("input_data_path = ", input_data_path)
    print("requested_iov = ", requested_iov)
    print("expert_config = ", expert_config)

    if not os.path.exists('plots'):
        os.makedirs('plots')

    # Add underflow, overflow and fit result
    gStyle.SetOptStat(1111111)
    gStyle.SetOptFit(1111)

    # ------------------------------------------------------------------------
    # ------------------------------------------------------------------------
    # ------------------------------------------------------------------------
    # Define new validation histograms
    clusterTime_histMeanStdDev_CrystalCalibBlocksBhabhaVal = TH1F(
        "clusterTime_histMeanStdDev_CrystalCalibBlocksBhabhaVal",
        "Bhabha: Cluster t means;Block of runs with one crystal calibration;Mean elec ECL cluster time / run block.  Err = RMS",
        30, 1, 30)
    peakClusterTimesGoodFit_histMeanStdDev_CrystalCalibBlocksBhabhaVal = TH1F(
        "peakClusterTimesGoodFit_histMeanStdDev_CrystalCalibBlocksBhabhaVal",
        "Bhabha: Mean(cluster t fit means);Runs of const crys calib;Mean(elec time fit mean / crys)/run block. Err=fit sigma",
        30, 1, 30)

    clusterTime_histMeanStdDev_CrystalCalibBlocksHadronVal = TH1F(
        "clusterTime_histMeanStdDev_CrystalCalibBlocksHadronVal",
        "Hadron: Cluster t means;Block of runs with one crystal calibration;Mean photon ECL cluster time / run block.  Err = RMS",
        30, 1, 30)
    peakClusterTimesGoodFit_histMeanStdDev_CrystalCalibBlocksHadronVal = TH1F(
        "peakClusterTimesGoodFit_histMeanStdDev_CrystalCalibBlocksHadronVal",
        "Hadron: Mean(cluster t fit means);Runs of const crys calib;Mean(photon time fit mean / crys)/run block. Err=fit sigma",
        30, 1, 30)

    myC = TCanvas("myC")

    # ------------------------------------------------------------------------
    # ------------------------------------------------------------------------
    # ------------------------------------------------------------------------
    # Crate calibrations
    print("\n---------------------------------------- \nBhabha crate calibrations: \n\n")
    crate_calib_output_dir = Path(job_path)
    crate_calib_files = sorted(crate_calib_output_dir.glob('**/eclBhabhaTAlgorithm_cratesAllRuns.root'))

    print("List of crate calibration files:\n")
    print(crate_calib_files)

    num_files = len(crate_calib_files)
    print(f'Looping over {num_files} files')
    for count, in_file_name in enumerate(crate_calib_files, start=1):
        in_file = r.TFile(str(in_file_name))
        print("--------------------\nReading file ", in_file, "\n Crate calib iteration = ", count, "\n")

        inFileBaseName = str(in_file_name)
        inFileBaseName = os.path.basename(inFileBaseName)
        inFileBaseName = inFileBaseName[:-5]
        print("inFileBaseName = ", inFileBaseName)

        # ECLcrateTimeCalibration_physics_#, where # sets the iteration
        crate_calib_directory_iter = os.path.split(os.path.split(os.path.split(os.path.dirname(in_file_name))[0])[0])[1]

        # Plots to read in and save as pdfs
        tcrateNew_MINUS_tcrateOld_allRuns = in_file.Get("tcrateNew_MINUS_tcrateOld_allRuns")
        tcrateNew_MINUS_tcrateOld_allRuns_allCrates = in_file.Get("tcrateNew_MINUS_tcrateOld_allRuns_allCrates")

        ###############################
        tcrateNew_MINUS_tcrateOld_allRuns.Draw("")
        tcrateNew_MINUS_tcrateOld_allRuns__Filename = str(
            "plots/" +
            crate_calib_directory_iter +
            "__" +
            "tcrateNew_MINUS_tcrateOld_allRuns__" +
            inFileBaseName +
            "__counter" +
            str(count) +
            ".pdf")
        print("tcrateNew_MINUS_tcrateOld_allRuns__Filename = ", tcrateNew_MINUS_tcrateOld_allRuns__Filename)
        myC.Print(tcrateNew_MINUS_tcrateOld_allRuns__Filename)

        myC.SetLogy(1)
        tcrateNew_MINUS_tcrateOld_allRuns.Draw("")
        tcrateNew_MINUS_tcrateOld_allRunsLogY__Filename = str("plots/" + crate_calib_directory_iter + "__" +
                                                              "tcrateNew_MINUS_tcrateOld_allRuns__logY__" +
                                                              inFileBaseName + "__counter" + str(count) + ".pdf")
        print("tcrateNew_MINUS_tcrateOld_allRunsLogY__Filename = ", tcrateNew_MINUS_tcrateOld_allRunsLogY__Filename)
        myC.Print(tcrateNew_MINUS_tcrateOld_allRunsLogY__Filename)
        myC.SetLogy(0)

        ###############################
        tcrateNew_MINUS_tcrateOld_allRuns_allCrates.Draw("")
        tcrateNew_MINUS_tcrateOld_allRuns_allCrates__Filename = str("plots/" + crate_calib_directory_iter + "__" +
                                                                    "tcrateNew_MINUS_tcrateOld_allRuns_allCrates__" +
                                                                    inFileBaseName + "__counter" + str(count) + ".pdf")
        print("tcrateNew_MINUS_tcrateOld_allRuns_allCrates__Filename = ", tcrateNew_MINUS_tcrateOld_allRuns_allCrates__Filename)
        myC.Print(tcrateNew_MINUS_tcrateOld_allRuns_allCrates__Filename)

        myC.SetLogy(1)
        tcrateNew_MINUS_tcrateOld_allRuns_allCrates.Draw("")
        tcrateNew_MINUS_tcrateOld_allRuns_allCratesLogY__Filename = str(
            "plots/" +
            crate_calib_directory_iter +
            "__" +
            "tcrateNew_MINUS_tcrateOld_allRuns_allCrates__logY__" +
            inFileBaseName +
            "__counter" +
            str(count) +
            ".pdf")
        print("tcrateNew_MINUS_tcrateOld_allRuns_allCratesLogY__Filename = ",
              tcrateNew_MINUS_tcrateOld_allRuns_allCratesLogY__Filename)
        myC.Print(tcrateNew_MINUS_tcrateOld_allRuns_allCratesLogY__Filename)
        myC.SetLogy(0)

    # ------------------------------------------------------------------------
    # ------------------------------------------------------------------------
    # ------------------------------------------------------------------------
    # Crystal calibrations - before the crystal merger
    print("\n---------------------------------------- \nBhabha crystal calibrations: \n\n")
    crystal_calib_output_dir = Path(job_path)
    crystal_calib_files = sorted(crystal_calib_output_dir.glob(
        '**/ECLcrystalTimeCalibration_physics_*/*/*/eclBhabhaTAlgorithm_*.root'))

    print("List of crystal calibration files:\n")
    print(crystal_calib_files)

    num_files = len(crystal_calib_files)
    print(f'Looping over {num_files} files')
    for count, in_file_name in enumerate(crystal_calib_files, start=1):
        in_file = r.TFile(str(in_file_name))
        print("--------------------\nReading file ", in_file, "\n crystal calib iteration = ", count, "\n")

        inFileBaseName = str(in_file_name)
        inFileBaseName = os.path.basename(inFileBaseName)
        inFileBaseName = inFileBaseName[:-5]
        print("inFileBaseName = ", inFileBaseName)

        # ECLcrystalTimeCalibration_physics_#, where # sets the iteration
        crystal_calib_directory_iter = os.path.split(os.path.split(os.path.split(os.path.dirname(in_file_name))[0])[0])[1]

        # Plots to read in and save as pdfs
        tsNew_MINUS_tsOld__cid = in_file.Get("tsNew_MINUS_tsOld__cid")
        tsNew_MINUS_tsOld = in_file.Get("tsNew_MINUS_tsOld")
        tsNew_MINUS_tsCustomPrev__cid = in_file.Get("tsNew_MINUS_tsCustomPrev__cid")
        tsNew_MINUS_tsCustomPrev = in_file.Get("tsNew_MINUS_tsCustomPrev")
        tsNew_MINUS_tsOldBhabha__cid = in_file.Get("tsNew_MINUS_tsOldBhabha__cid")
        tsNew_MINUS_tsOldBhabha = in_file.Get("tsNew_MINUS_tsOldBhabha")
        cutflow = in_file.Get("cutflow")
        h_crysHits = in_file.Get("h_crysHits")

        ###############################
        tsNew_MINUS_tsOld__cid.Draw("")
        tsNew_MINUS_tsOld__cid.SetStats(0)
        tsNew_MINUS_tsOld__cid.Draw("")
        tsNew_MINUS_tsOld__cid__Filename = str("plots/" + crystal_calib_directory_iter + "__" + "tsNew_MINUS_tsOld__cid__" +
                                               inFileBaseName + "__counter" + str(count) + ".pdf")
        print("tsNew_MINUS_tsOld__cid__Filename = ", tsNew_MINUS_tsOld__cid__Filename)
        myC.Print(tsNew_MINUS_tsOld__cid__Filename)

        ###############################
        tsNew_MINUS_tsOld.Draw("")
        tsNew_MINUS_tsOld__Filename = str("plots/" + crystal_calib_directory_iter + "__" + "tsNew_MINUS_tsOld__" +
                                          inFileBaseName + "__counter" + str(count) + ".pdf")
        print("tsNew_MINUS_tsOld__Filename = ", tsNew_MINUS_tsOld__Filename)
        myC.Print(tsNew_MINUS_tsOld__Filename)

        myC.SetLogy(1)
        tsNew_MINUS_tsOld.Draw("")
        tsNew_MINUS_tsOldLogY__Filename = str("plots/" + crystal_calib_directory_iter + "__" + "tsNew_MINUS_tsOld__logY__" +
                                              inFileBaseName + "__counter" + str(count) + ".pdf")
        print("tsNew_MINUS_tsOldLogY__Filename = ", tsNew_MINUS_tsOldLogY__Filename)
        myC.Print(tsNew_MINUS_tsOldLogY__Filename)
        myC.SetLogy(0)

        ###############################
        tsNew_MINUS_tsCustomPrev__cid.Draw("")
        tsNew_MINUS_tsCustomPrev__cid.SetStats(0)
        tsNew_MINUS_tsCustomPrev__cid.Draw("")
        tsNew_MINUS_tsCustomPrev__cid__Filename = str(
            "plots/" +
            crystal_calib_directory_iter +
            "__" +
            "tsNew_MINUS_tsCustomPrev__cid__" +
            inFileBaseName +
            "__counter" +
            str(count) +
            ".pdf")
        print("tsNew_MINUS_tsCustomPrev__cid__Filename = ", tsNew_MINUS_tsCustomPrev__cid__Filename)
        myC.Print(tsNew_MINUS_tsCustomPrev__cid__Filename)

        ###############################
        tsNew_MINUS_tsCustomPrev.Draw("")
        tsNew_MINUS_tsCustomPrev__Filename = str("plots/" + crystal_calib_directory_iter + "__" + "tsNew_MINUS_tsCustomPrev__" +
                                                 inFileBaseName + "__counter" + str(count) + ".pdf")
        print("tsNew_MINUS_tsCustomPrev__Filename = ", tsNew_MINUS_tsCustomPrev__Filename)
        myC.Print(tsNew_MINUS_tsCustomPrev__Filename)

        myC.SetLogy(1)
        tsNew_MINUS_tsCustomPrev.Draw("")
        tsNew_MINUS_tsCustomPrevLogY__Filename = str(
            "plots/" +
            crystal_calib_directory_iter +
            "__" +
            "tsNew_MINUS_tsCustomPrev__logY__" +
            inFileBaseName +
            "__counter" +
            str(count) +
            ".pdf")
        print("tsNew_MINUS_tsCustomPrevLogY__Filename = ", tsNew_MINUS_tsCustomPrevLogY__Filename)
        myC.Print(tsNew_MINUS_tsCustomPrevLogY__Filename)
        myC.SetLogy(0)

        ###############################
        tsNew_MINUS_tsOldBhabha__cid.Draw("")
        tsNew_MINUS_tsOldBhabha__cid.SetStats(0)
        tsNew_MINUS_tsOldBhabha__cid.Draw("")
        tsNew_MINUS_tsOldBhabha__cid__Filename = str(
            "plots/" +
            crystal_calib_directory_iter +
            "__" +
            "tsNew_MINUS_tsOldBhabha__cid__" +
            inFileBaseName +
            "__counter" +
            str(count) +
            ".pdf")
        print("tsNew_MINUS_tsOldBhabha__cid__Filename = ", tsNew_MINUS_tsOldBhabha__cid__Filename)
        myC.Print(tsNew_MINUS_tsOldBhabha__cid__Filename)

        ###############################
        tsNew_MINUS_tsOldBhabha.Draw("")
        tsNew_MINUS_tsOldBhabha__Filename = str("plots/" + crystal_calib_directory_iter + "__" + "tsNew_MINUS_tsOldBhabha__" +
                                                inFileBaseName + "__counter" + str(count) + ".pdf")
        print("tsNew_MINUS_tsOldBhabha__Filename = ", tsNew_MINUS_tsOldBhabha__Filename)
        myC.Print(tsNew_MINUS_tsOldBhabha__Filename)

        myC.SetLogy(1)
        tsNew_MINUS_tsOldBhabha.Draw("")
        tsNew_MINUS_tsOldBhabhaLogY__Filename = str(
            "plots/" +
            crystal_calib_directory_iter +
            "__" +
            "tsNew_MINUS_tsOldBhabha__logY__" +
            inFileBaseName +
            "__counter" +
            str(count) +
            ".pdf")
        print("tsNew_MINUS_tsOldBhabhaLogY__Filename = ", tsNew_MINUS_tsOldBhabhaLogY__Filename)
        myC.Print(tsNew_MINUS_tsOldBhabhaLogY__Filename)
        myC.SetLogy(0)

        ###############################
        cutflow.Draw("")
        cutflow__Filename = str("plots/" + crystal_calib_directory_iter + "__" + "cutflow__" +
                                inFileBaseName + "__counter" + str(count) + ".pdf")
        print("cutflow__Filename = ", cutflow__Filename)
        myC.Print(cutflow__Filename)

        ###############################
        myC.SetLogy(1)
        h_crysHits.Draw("")
        h_crysHits.SetStats(0)
        h_crysHits.Draw("")
        h_crysHits__Filename = str("plots/" + crystal_calib_directory_iter + "__" + "h_crysHits__" +
                                   inFileBaseName + "__counter" + str(count) + ".pdf")
        print("h_crysHits__Filename = ", h_crysHits__Filename)
        myC.Print(h_crysHits__Filename)
        myC.SetLogy(0)

    # ------------------------------------------------------------------------
    # ------------------------------------------------------------------------
    # ------------------------------------------------------------------------
    # crystal mergers
    print("\n---------------------------------------- \nBhabha calibration crystal mergers: \n\n")
    merger_alg_output_dir = Path(job_path)
    merger_files = sorted(merger_alg_output_dir.glob('**/ECLCrystalTimeOffsetMerged_*.root'))

    print("List of bhabha validation files:\n")
    print(merger_files)

    num_files = len(merger_files)
    print(f'Looping over {num_files} files')
    for count, in_file_name in enumerate(merger_files, start=1):
        in_file = r.TFile(str(in_file_name))
        print("--------------------\nReading file ", in_file, "\n Crystal calib iteration or blocks = ", count, "\n")

        inFileBaseName = str(in_file_name)
        inFileBaseName = os.path.basename(inFileBaseName)
        inFileBaseName = inFileBaseName[:-5]
        print("inFileBaseName = ", inFileBaseName)

        # ecl_t_merge_#, where # sets the iteration
        merge_directory_iter = os.path.split(os.path.split(os.path.split(os.path.dirname(in_file_name))[0])[0])[1]

        # Plots to read in and save as pdfs
        newBhabhaMinusCustomPrev__cid = in_file.Get("newBhabhaMinusCustomPrev__cid")
        newBhabhaMinusCustomPrevBhabha__cid = in_file.Get("newBhabhaMinusCustomPrevBhabha__cid")
        TsNewBhabha_MINUS_TsCustomPrev = in_file.Get("TsNewBhabha_MINUS_TsCustomPrev")
        TsNewBhabha_MINUS_TsCustomPrevBhabha = in_file.Get("TsNewBhabha_MINUS_TsCustomPrevBhabha")

        ###############################
        newBhabhaMinusCustomPrev__cid.Draw("")
        newBhabhaMinusCustomPrev__cid.SetStats(0)
        newBhabhaMinusCustomPrev__cid.Draw("")
        newBhabhaMinusCustomPrev__cid__Filename = str("plots/" + merge_directory_iter + "__" + "newBhabhaMinusCustomPrev__cid__" +
                                                      inFileBaseName + "__counter" + str(count) + ".pdf")
        print("newBhabhaMinusCustomPrev__cid__Filename = ", newBhabhaMinusCustomPrev__cid__Filename)
        myC.Print(newBhabhaMinusCustomPrev__cid__Filename)

        ###############################
        newBhabhaMinusCustomPrevBhabha__cid.Draw("")
        newBhabhaMinusCustomPrevBhabha__cid.SetStats(0)
        newBhabhaMinusCustomPrevBhabha__cid.Draw("")
        newBhabhaMinusCustomPrevBhabha__cid__Filename = str(
            "plots/" +
            merge_directory_iter +
            "__" +
            "newBhabhaMinusCustomPrevBhabha__cid__" +
            inFileBaseName +
            "__counter" +
            str(count) +
            ".pdf")
        print("newBhabhaMinusCustomPrevBhabha__cid__Filename = ", newBhabhaMinusCustomPrevBhabha__cid__Filename)
        myC.Print(newBhabhaMinusCustomPrevBhabha__cid__Filename)

        ###############################
        TsNewBhabha_MINUS_TsCustomPrev.Draw("")
        TsNewBhabha_MINUS_TsCustomPrev__Filename = str("plots/" + merge_directory_iter + "__" + "TsNewBhabha_MINUS_TsCustomPrev__" +
                                                       inFileBaseName + "__counter" + str(count) + ".pdf")
        print("TsNewBhabha_MINUS_TsCustomPrev__Filename = ", TsNewBhabha_MINUS_TsCustomPrev__Filename)
        myC.Print(TsNewBhabha_MINUS_TsCustomPrev__Filename)

        myC.SetLogy(1)
        TsNewBhabha_MINUS_TsCustomPrev.Draw("")
        TsNewBhabha_MINUS_TsCustomPrevLogY__Filename = str(
            "plots/" +
            merge_directory_iter +
            "__" +
            "TsNewBhabha_MINUS_TsCustomPrev__logY__" +
            inFileBaseName +
            "__counter" +
            str(count) +
            ".pdf")
        print("TsNewBhabha_MINUS_TsCustomPrevLogY__Filename = ", TsNewBhabha_MINUS_TsCustomPrevLogY__Filename)
        myC.Print(TsNewBhabha_MINUS_TsCustomPrevLogY__Filename)
        myC.SetLogy(0)

        ###############################
        TsNewBhabha_MINUS_TsCustomPrevBhabha.Draw("")
        TsNewBhabha_MINUS_TsCustomPrevBhabha__Filename = str(
            "plots/" +
            merge_directory_iter +
            "__" +
            "TsNewBhabha_MINUS_TsCustomPrevBhabha__" +
            inFileBaseName +
            "__counter" +
            str(count) +
            ".pdf")
        print("TsNewBhabha_MINUS_TsCustomPrevBhabha__Filename = ", TsNewBhabha_MINUS_TsCustomPrevBhabha__Filename)
        myC.Print(TsNewBhabha_MINUS_TsCustomPrevBhabha__Filename)

        myC.SetLogy(1)
        TsNewBhabha_MINUS_TsCustomPrevBhabha.Draw("")
        TsNewBhabha_MINUS_TsCustomPrevBhabhaLogY__Filename = str("plots/" + merge_directory_iter +
                                                                 "__" + "TsNewBhabha_MINUS_TsCustomPrevBhabha__logY__" +
                                                                 inFileBaseName + "__counter" + str(count) + ".pdf")
        print("TsNewBhabha_MINUS_TsCustomPrevBhabhaLogY__Filename = ", TsNewBhabha_MINUS_TsCustomPrevBhabhaLogY__Filename)
        myC.Print(TsNewBhabha_MINUS_TsCustomPrevBhabhaLogY__Filename)
        myC.SetLogy(0)

    # ------------------------------------------------------------------------
    # ------------------------------------------------------------------------
    # ------------------------------------------------------------------------
    # bhabha self-consistency
    print("\n---------------------------------------- \nBhabha self-consistency check: \n\n")
    bhabhaVal_alg_output_dir = Path(job_path) / 'ECLcrystalTimeCalValidation_bhabhaPhysics/0/algorithm_output/'
    bhabhaVal_files = sorted(bhabhaVal_alg_output_dir.glob('**/eclBhabhaTValidationAlgorithm_*.root'))

    print("List of bhabha validation files:\n")
    print(bhabhaVal_files)

    num_files = len(bhabhaVal_files)
    print(f'Looping over {num_files} files')
    for count, in_file_name in enumerate(bhabhaVal_files, start=1):
        in_file = r.TFile(str(in_file_name))
        print("--------------------\nReading file ", in_file, ", crystal calib block # = ", count, "\n")

        inFileBaseName = str(in_file_name)
        inFileBaseName = os.path.basename(inFileBaseName)
        inFileBaseName = inFileBaseName[:-5]
        print("inFileBaseName = ", inFileBaseName)

        bhabhaVal_directory = "ECLcrystalTimeCalValidation_bhabhaPhysics"

        # Read in the plots
        peakClusterTimesGoodFit = in_file.Get("peakClusterTimesGoodFit")
        clusterTime = in_file.Get("clusterTime")

        # Analyse the straight up cluster time histogram
        title = "From: " + inFileBaseName
        title = title + " : cluster time" + ", block " + str(count)
        clusterTime.SetTitle(title)
        clusterTime.Draw("")
        clusterTime.Fit("gaus")

        clusterTimeFilename = str("plots/" + inFileBaseName + "__clusterTime__block" + str(count) + ".pdf")
        print("clusterTimeFilename = ", clusterTimeFilename)
        myC.Print(clusterTimeFilename)

        clusterTime_histMeanStdDev_CrystalCalibBlocksBhabhaVal.SetBinContent(count, clusterTime.GetMean())
        clusterTime_histMeanStdDev_CrystalCalibBlocksBhabhaVal.SetBinError(count, clusterTime.GetStdDev())

        # Analyse the histogram of the mean of the fit to the cluster times
        title = "From: " + inFileBaseName
        title = title + " : cluster time fits" + ", block " + str(count)
        peakClusterTimesGoodFit.SetTitle(title)
        peakClusterTimesGoodFit.Draw("")
        peakClusterTimesGoodFit.Fit("gaus")

        peakClusterTimesGoodFitFilename = str("plots/" + inFileBaseName + "__peakClusterTimesGoodFit__block" + str(count) + ".pdf")
        print("peakClusterTimesGoodFitFilename = ", peakClusterTimesGoodFitFilename)
        myC.Print(peakClusterTimesGoodFitFilename)

        peakClusterTimesGoodFit_histMeanStdDev_CrystalCalibBlocksBhabhaVal.SetBinContent(count, peakClusterTimesGoodFit.GetMean())
        peakClusterTimesGoodFit_histMeanStdDev_CrystalCalibBlocksBhabhaVal.SetBinError(count, peakClusterTimesGoodFit.GetStdDev())

        # Plots to read in and save as pdfs
        tsNew_MINUS_tsCustomPrev__cid = in_file.Get("tsNew_MINUS_tsCustomPrev__cid")
        tsNew_MINUS_tsCustomPrev = in_file.Get("tsNew_MINUS_tsCustomPrev")
        clusterTimePeak_ClusterEnergy_varBin = in_file.Get("clusterTimePeak_ClusterEnergy_varBin")
        peakClusterTime_cid = in_file.Get("peakClusterTime_cid")
        peakClusterTimes = in_file.Get("peakClusterTimes")
        g_clusterTimeFractionInWindowInGoodECLRings__cid = in_file.Get("g_clusterTimeFractionInWindowInGoodECLRings__cid")
        clusterTimeFractionInWindow = in_file.Get("clusterTimeFractionInWindow")

        ###############################
        tsNew_MINUS_tsCustomPrev__cid.Draw("")
        tsNew_MINUS_tsCustomPrev__cid.SetStats(0)
        tsNew_MINUS_tsCustomPrev__cid.Draw("")
        tsNew_MINUS_tsCustomPrev__cid__Filename = str("plots/" + bhabhaVal_directory + "__" + "tsNew_MINUS_tsCustomPrev__cid__" +
                                                      inFileBaseName + "__block" + str(count) + ".pdf")
        print("tsNew_MINUS_tsCustomPrev__cid__Filename = ", tsNew_MINUS_tsCustomPrev__cid__Filename)
        myC.Print(tsNew_MINUS_tsCustomPrev__cid__Filename)

        ###############################
        tsNew_MINUS_tsCustomPrev.Draw("")
        tsNew_MINUS_tsCustomPrev__Filename = str("plots/" + bhabhaVal_directory + "__" + "tsNew_MINUS_tsCustomPrev__" +
                                                 inFileBaseName + "__block" + str(count) + ".pdf")
        print("tsNew_MINUS_tsCustomPrev__Filename = ", tsNew_MINUS_tsCustomPrev__Filename)
        myC.Print(tsNew_MINUS_tsCustomPrev__Filename)

        myC.SetLogy(1)
        tsNew_MINUS_tsCustomPrev.Draw("")
        tsNew_MINUS_tsCustomPrevLogY__Filename = str("plots/" + bhabhaVal_directory + "__" + "tsNew_MINUS_tsCustomPrev__logY__" +
                                                     inFileBaseName + "__block" + str(count) + ".pdf")
        print("tsNew_MINUS_tsCustomPrevLogY__Filename = ", tsNew_MINUS_tsCustomPrevLogY__Filename)
        myC.Print(tsNew_MINUS_tsCustomPrevLogY__Filename)
        myC.SetLogy(0)

        ###############################
        clusterTimePeak_ClusterEnergy_varBin.Draw("")
        clusterTimePeak_ClusterEnergy_varBin.SetStats(0)
        clusterTimePeak_ClusterEnergy_varBin.Draw("")
        clusterTimePeak_ClusterEnergy_varBin.SetMinimum(-3)
        clusterTimePeak_ClusterEnergy_varBin.SetMaximum(3)
        clusterTimePeak_ClusterEnergy_varBinLogX__Filename = str("plots/" + bhabhaVal_directory +
                                                                 "__" + "clusterTimePeak_ClusterEnergy_varBin__logX__" +
                                                                 inFileBaseName + "__block" + str(count) + ".pdf")
        print("clusterTimePeak_ClusterEnergy_varBinLogX__Filename = ", clusterTimePeak_ClusterEnergy_varBinLogX__Filename)
        myC.Print(clusterTimePeak_ClusterEnergy_varBinLogX__Filename)

        ###############################
        peakClusterTime_cid.Draw("")
        peakClusterTime_cid.SetStats(0)
        peakClusterTime_cid.Draw("")
        peakClusterTime_cid__Filename = str("plots/" + bhabhaVal_directory + "__" + "peakClusterTime_cid__" +
                                            inFileBaseName + "__block" + str(count) + ".pdf")
        print("peakClusterTime_cid__Filename = ", peakClusterTime_cid__Filename)
        myC.Print(peakClusterTime_cid__Filename)

        ###############################
        peakClusterTimes.Draw("")
        peakClusterTimes__Filename = str("plots/" + bhabhaVal_directory + "__" + "peakClusterTimes__" +
                                         inFileBaseName + "__block" + str(count) + ".pdf")
        print("peakClusterTimes__Filename = ", peakClusterTimes__Filename)
        myC.Print(peakClusterTimes__Filename)

        ###############################
        g_clusterTimeFractionInWindowInGoodECLRings__cid.Draw("")
        g_clusterTimeFractionInWindowInGoodECLRings__cid__Filename = str(
            "plots/" +
            bhabhaVal_directory +
            "__" +
            "g_clusterTimeFractionInWindowInGoodECLRings__cid__" +
            inFileBaseName +
            "__block" +
            str(count) +
            ".pdf")
        print("g_clusterTimeFractionInWindowInGoodECLRings__cid__Filename = ",
              g_clusterTimeFractionInWindowInGoodECLRings__cid__Filename)
        myC.Print(g_clusterTimeFractionInWindowInGoodECLRings__cid__Filename)

        ###############################
        clusterTimeFractionInWindow.Draw("")
        clusterTimeFractionInWindow__Filename = str("plots/" + bhabhaVal_directory + "__" + "clusterTimeFractionInWindow__" +
                                                    inFileBaseName + "__block" + str(count) + ".pdf")
        print("clusterTimeFractionInWindow__Filename = ", clusterTimeFractionInWindow__Filename)
        myC.Print(clusterTimeFractionInWindow__Filename)

    clusterTime_histMeanStdDev_CrystalCalibBlocksBhabhaVal.Draw("")
    myC.Print("plots/crystalCalibBlocksBhabhaVal__clusterTime_histMeanStdDev.pdf")

    peakClusterTimesGoodFit_histMeanStdDev_CrystalCalibBlocksBhabhaVal.Draw("")
    myC.Print("plots/crystalCalibBlocksBhabhaVal__peakClusterTimesGoodFit_histMeanStdDev.pdf")

    # ------------------------------------------------------------------------
    # ------------------------------------------------------------------------
    # ------------------------------------------------------------------------
    # Hadron validation
    print("\n---------------------------------------- \nHadron self-consistency check: \n\n")
    hadronVal_alg_output_dir = Path(job_path) / 'ECLcrystalTimeCalValidation_hadronPhysics/0/algorithm_output/'
    hadronVal_files = sorted(hadronVal_alg_output_dir.glob('**/eclHadronTValidationAlgorithm_*.root'))

    print("List of hadron validation files:\n")
    print(hadronVal_files)

    num_files = len(hadronVal_files)
    print(f'Looping over {num_files} files')
    for count, in_file_name in enumerate(hadronVal_files, start=1):
        in_file = r.TFile(str(in_file_name))
        print("--------------------\nReading file ", in_file, ", crystal calib block # = ", count, "\n")

        inFileBaseName = str(in_file_name)
        inFileBaseName = os.path.basename(inFileBaseName)
        inFileBaseName = inFileBaseName[:-5]
        print("inFileBaseName = ", inFileBaseName)

        hadVal_directory = "ECLcrystalTimeCalValidation_hadronPhysics"

        # Read in the plots
        peakClusterTimesGoodFit = in_file.Get("peakClusterTimesGoodFit")
        clusterTime = in_file.Get("clusterTime")

        # Analyse the straight up cluster time histogram
        title = "From: " + inFileBaseName
        title = title + " : cluster time" + ", block " + str(count)
        clusterTime.SetTitle(title)
        clusterTime.Draw("")
        clusterTime.Fit("gaus")

        clusterTimeFilename = str("plots/" + inFileBaseName + "__clusterTime__block" + str(count) + ".pdf")
        print("clusterTimeFilename = ", clusterTimeFilename)
        myC.Print(clusterTimeFilename)

        clusterTime_histMeanStdDev_CrystalCalibBlocksHadronVal.SetBinContent(count, clusterTime.GetMean())
        clusterTime_histMeanStdDev_CrystalCalibBlocksHadronVal.SetBinError(count, clusterTime.GetStdDev())

        # Analyse the histogram of the mean of the fit to the cluster times
        title = "From: " + inFileBaseName
        title = title + " : cluster time fits" + ", block " + str(count)
        peakClusterTimesGoodFit.SetTitle(title)
        peakClusterTimesGoodFit.Draw("")
        peakClusterTimesGoodFit.Fit("gaus")

        peakClusterTimesGoodFitFilename = str("plots/" + inFileBaseName + "__peakClusterTimesGoodFit __block" + str(count) + ".pdf")
        print("peakClusterTimesGoodFitFilename = ", peakClusterTimesGoodFitFilename)
        myC.Print(peakClusterTimesGoodFitFilename)

        peakClusterTimesGoodFit_histMeanStdDev_CrystalCalibBlocksHadronVal.SetBinContent(count, peakClusterTimesGoodFit.GetMean())
        peakClusterTimesGoodFit_histMeanStdDev_CrystalCalibBlocksHadronVal.SetBinError(count, peakClusterTimesGoodFit.GetStdDev())

        # Plots to read in and save as pdfs
        tsNew_MINUS_tsCustomPrev__cid = in_file.Get("tsNew_MINUS_tsCustomPrev__cid")
        tsNew_MINUS_tsCustomPrev = in_file.Get("tsNew_MINUS_tsCustomPrev")
        clusterTimePeak_ClusterEnergy_varBin = in_file.Get("clusterTimePeak_ClusterEnergy_varBin")
        peakClusterTime_cid = in_file.Get("peakClusterTime_cid")
        peakClusterTimes = in_file.Get("peakClusterTimes")
        g_clusterTimeFractionInWindowInGoodECLRings__cid = in_file.Get("g_clusterTimeFractionInWindowInGoodECLRings__cid")
        clusterTimeFractionInWindow = in_file.Get("clusterTimeFractionInWindow")

        ###############################
        tsNew_MINUS_tsCustomPrev__cid.Draw("")
        tsNew_MINUS_tsCustomPrev__cid.SetStats(0)
        tsNew_MINUS_tsCustomPrev__cid.Draw("")
        tsNew_MINUS_tsCustomPrev__cid__Filename = str("plots/" + hadVal_directory + "__" + "tsNew_MINUS_tsCustomPrev__cid__" +
                                                      inFileBaseName + "__block" + str(count) + ".pdf")
        print("tsNew_MINUS_tsCustomPrev__cid__Filename = ", tsNew_MINUS_tsCustomPrev__cid__Filename)
        myC.Print(tsNew_MINUS_tsCustomPrev__cid__Filename)

        ###############################
        tsNew_MINUS_tsCustomPrev.Draw("")
        tsNew_MINUS_tsCustomPrev__Filename = str("plots/" + hadVal_directory + "__" + "tsNew_MINUS_tsCustomPrev__" +
                                                 inFileBaseName + "__block" + str(count) + ".pdf")
        print("tsNew_MINUS_tsCustomPrev__Filename = ", tsNew_MINUS_tsCustomPrev__Filename)
        myC.Print(tsNew_MINUS_tsCustomPrev__Filename)

        myC.SetLogy(1)
        tsNew_MINUS_tsCustomPrev.Draw("")
        tsNew_MINUS_tsCustomPrevLogY__Filename = str("plots/" + hadVal_directory + "__" + "tsNew_MINUS_tsCustomPrev__logY__" +
                                                     inFileBaseName + "__block" + str(count) + ".pdf")
        print("tsNew_MINUS_tsCustomPrevLogY__Filename = ", tsNew_MINUS_tsCustomPrevLogY__Filename)
        myC.Print(tsNew_MINUS_tsCustomPrevLogY__Filename)
        myC.SetLogy(0)

        ###############################
        myC.SetLogx(1)
        clusterTimePeak_ClusterEnergy_varBin.Draw("")
        clusterTimePeak_ClusterEnergy_varBin.SetStats(0)
        clusterTimePeak_ClusterEnergy_varBin.Draw("")
        clusterTimePeak_ClusterEnergy_varBin.SetMinimum(-10)
        clusterTimePeak_ClusterEnergy_varBin.SetMaximum(5)
        clusterTimePeak_ClusterEnergy_varBinLogX__Filename = str("plots/" + hadVal_directory +
                                                                 "__" + "clusterTimePeak_ClusterEnergy_varBin__logX__" +
                                                                 inFileBaseName + "__block" + str(count) + ".pdf")
        print("clusterTimePeak_ClusterEnergy_varBinLogX__Filename = ", clusterTimePeak_ClusterEnergy_varBinLogX__Filename)
        myC.Print(clusterTimePeak_ClusterEnergy_varBinLogX__Filename)
        myC.SetLogx(0)

        ###############################
        peakClusterTime_cid.Draw("")
        peakClusterTime_cid.SetStats(0)
        peakClusterTime_cid.Draw("")
        peakClusterTime_cid__Filename = str("plots/" + hadVal_directory + "__" + "peakClusterTime_cid__" +
                                            inFileBaseName + "__block" + str(count) + ".pdf")
        print("peakClusterTime_cid__Filename = ", peakClusterTime_cid__Filename)
        myC.Print(peakClusterTime_cid__Filename)

        ###############################
        peakClusterTimes.Draw("")
        peakClusterTimes__Filename = str("plots/" + hadVal_directory + "__" + "peakClusterTimes__" +
                                         inFileBaseName + "__block" + str(count) + ".pdf")
        print("peakClusterTimes__Filename = ", peakClusterTimes__Filename)
        myC.Print(peakClusterTimes__Filename)

        ###############################
        g_clusterTimeFractionInWindowInGoodECLRings__cid.Draw("")
        g_clusterTimeFractionInWindowInGoodECLRings__cid__Filename = str(
            "plots/" +
            hadVal_directory +
            "__" +
            "g_clusterTimeFractionInWindowInGoodECLRings__cid__" +
            inFileBaseName +
            "__block" +
            str(count) +
            ".pdf")
        print("g_clusterTimeFractionInWindowInGoodECLRings__cid__Filename = ",
              g_clusterTimeFractionInWindowInGoodECLRings__cid__Filename)
        myC.Print(g_clusterTimeFractionInWindowInGoodECLRings__cid__Filename)

        ###############################
        clusterTimeFractionInWindow.Draw("")
        clusterTimeFractionInWindow__Filename = str("plots/" + hadVal_directory + "__" + "clusterTimeFractionInWindow__" +
                                                    inFileBaseName + "__block" + str(count) + ".pdf")
        print("clusterTimeFractionInWindow__Filename = ", clusterTimeFractionInWindow__Filename)
        myC.Print(clusterTimeFractionInWindow__Filename)

    clusterTime_histMeanStdDev_CrystalCalibBlocksHadronVal.Draw("")
    myC.Print("plots/crystalCalibBlocksHadronVal__clusterTime_histMeanStdDev.pdf")

    peakClusterTimesGoodFit_histMeanStdDev_CrystalCalibBlocksHadronVal.Draw("")
    myC.Print("plots/crystalCalibBlocksHadronVal__peakClusterTimesGoodFit_histMeanStdDev.pdf")

    # ------------------------------------------------------------------------
    # Write out histograms
    fout = TFile("ecl_timing_validateAirflow.root", "recreate")

    clusterTime_histMeanStdDev_CrystalCalibBlocksBhabhaVal.Write()
    peakClusterTimesGoodFit_histMeanStdDev_CrystalCalibBlocksBhabhaVal.Write()
    clusterTime_histMeanStdDev_CrystalCalibBlocksHadronVal.Write()
    peakClusterTimesGoodFit_histMeanStdDev_CrystalCalibBlocksHadronVal.Write()

    fout.Close()


if __name__ == "__main__":
    run_validation(*sys.argv[1:])
