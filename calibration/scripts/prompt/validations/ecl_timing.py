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
    # Validation histograms
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

        # Read in the plots
        peakClusterTimesGoodFit = in_file.Get("peakClusterTimesGoodFit")
        clusterTime = in_file.Get("clusterTime")

        # Analyse the straight up cluster time histogram
        title = "From: " + inFileBaseName
        title = title + " : cluster time" + ", block " + str(count)
        clusterTime.SetTitle(title)
        clusterTime.Draw("")
        clusterTime.Fit("gaus")

        clusterTimeFilename = str("plots/clusterTime__" + inFileBaseName + "__block" + str(count) + ".pdf")
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

        peakClusterTimesGoodFitFilename = str("plots/peakClusterTimesGoodFit__" + inFileBaseName + "__block" + str(count) + ".pdf")
        print("peakClusterTimesGoodFitFilename = ", peakClusterTimesGoodFitFilename)
        myC.Print(peakClusterTimesGoodFitFilename)

        peakClusterTimesGoodFit_histMeanStdDev_CrystalCalibBlocksBhabhaVal.SetBinContent(count, peakClusterTimesGoodFit.GetMean())
        peakClusterTimesGoodFit_histMeanStdDev_CrystalCalibBlocksBhabhaVal.SetBinError(count, peakClusterTimesGoodFit.GetStdDev())

    clusterTime_histMeanStdDev_CrystalCalibBlocksBhabhaVal.Draw("")
    myC.Print("plots/clusterTime_histMeanStdDev_CrystalCalibBlocksBhabhaVal.pdf")

    peakClusterTimesGoodFit_histMeanStdDev_CrystalCalibBlocksBhabhaVal.Draw("")
    myC.Print("plots/peakClusterTimesGoodFit_histMeanStdDev_CrystalCalibBlocksBhabhaVal.pdf")

    # ------------------------------------------------------------------------
    # Hadron self-consistency
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

        # Read in the plots
        peakClusterTimesGoodFit = in_file.Get("peakClusterTimesGoodFit")
        clusterTime = in_file.Get("clusterTime")

        # Analyse the straight up cluster time histogram
        title = "From: " + inFileBaseName
        title = title + " : cluster time" + ", block " + str(count)
        clusterTime.SetTitle(title)
        clusterTime.Draw("")
        clusterTime.Fit("gaus")

        clusterTimeFilename = str("plots/clusterTime__" + inFileBaseName + "__block" + str(count) + ".pdf")
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

        peakClusterTimesGoodFitFilename = str("plots/peakClusterTimesGoodFit__" + inFileBaseName + "__block" + str(count) + ".pdf")
        print("peakClusterTimesGoodFitFilename = ", peakClusterTimesGoodFitFilename)
        myC.Print(peakClusterTimesGoodFitFilename)

        peakClusterTimesGoodFit_histMeanStdDev_CrystalCalibBlocksHadronVal.SetBinContent(count, peakClusterTimesGoodFit.GetMean())
        peakClusterTimesGoodFit_histMeanStdDev_CrystalCalibBlocksHadronVal.SetBinError(count, peakClusterTimesGoodFit.GetStdDev())

    clusterTime_histMeanStdDev_CrystalCalibBlocksHadronVal.Draw("")
    myC.Print("plots/clusterTime_histMeanStdDev_CrystalCalibBlocksHadronVal.pdf")

    peakClusterTimesGoodFit_histMeanStdDev_CrystalCalibBlocksHadronVal.Draw("")
    myC.Print("plots/peakClusterTimesGoodFit_histMeanStdDev_CrystalCalibBlocksHadronVal.pdf")

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
