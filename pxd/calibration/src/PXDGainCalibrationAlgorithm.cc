/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/calibration/PXDGainCalibrationAlgorithm.h>
#include <pxd/dbobjects/PXDGainMapPar.h>
#include <vxd/dataobjects/VxdID.h>

#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>

#include <boost/format.hpp>
#include <cmath>


//ROOT
#include <TMath.h>
#include <TAxis.h>
#include <TMinuit.h>
#include <TRandom.h>

using namespace std;
using boost::format;
using namespace Belle2;


// Anonymous namespace for data objects used by both PXDGainCalibrationAlgorithm class and FCNGain function for MINUIT minimization.
namespace {

  std::shared_ptr<TTree> m_tree;

  /** SensorID of collected clusters */
  int m_sensorID;
  /** uCellID of collected clusters */
  int m_uCellID;
  /** vCellID of collected clusters */
  int m_vCellID;
  /** Signal in ADU of collected clusters */
  int m_signal;
  /** Flag for MC data  */
  bool m_isMC;

  /** Number of bins for cluster charge */
  int N = 20; // 32

  /** Noise for smearing cluster charge */
  float noise_sigma = 2.0;

  /** Lower edge of fit range */
  float fitRangeLower = 25;

  /** Upper edge of fit range */
  float fitRangeUpper = 170;

  /** Signal array for data */
  vector<float> countData(N, 0.0);

  /** Signal array for mc */
  vector<float> countMC(N, 0.0);

  /** Function to minimize in minuit fit. (chi2) */
  void FCNGain(int&, double* grad, double& f, double* p, int)
  {
    double gain = p[0];

    TAxis signalAxis(N, fitRangeLower, fitRangeUpper);
    int sumData = 0;
    int sumMC = 0;
    std::fill(countData.begin(), countData.end(), 0);
    std::fill(countMC.begin(), countMC.end(), 0);

    // Loop over the tree is to fill the signal histograms
    const auto nEntries = m_tree->GetEntries();
    for (int i = 0; i < nEntries; ++i) {
      m_tree->GetEntry(i);

      double noise = gRandom->Gaus(0.0, noise_sigma);
      if (m_isMC) {
        int bin = signalAxis.FindFixBin(gain * m_signal + noise) - 1;
        if (bin < N and bin >= 0) {
          countMC[bin] += 1;
          sumMC += 1;
        }
      } else {
        int bin = signalAxis.FindFixBin(m_signal + noise) - 1;
        if (bin < N and bin >= 0) {
          countData[bin] += 1;
          sumData += 1;
        }
      }
    }

    double chi2 = 0;
    // Computing chi2
    for (int i = 0; i < N; ++i) {
      if (countData[i] > 0 && countMC[i] > 0) {
        float sigmaData = 2.0 * std::sqrt(countData[i]) / sumData;
        float sigmaMC = 2.0 * std::sqrt(countMC[i]) / sumMC;
        float sigma2 = sigmaData * sigmaData + sigmaMC * sigmaMC;
        chi2 += std::pow(countData[i] / sumData - countMC[i] / sumMC, 2) / sigma2;
      }
    }
    f = chi2;
  }
}


PXDGainCalibrationAlgorithm::PXDGainCalibrationAlgorithm():
  CalibrationAlgorithm("PXDGainCollector"),
  minClusters(4000)
{
  setDescription(
    " -------------------------- PXDGainCalibrationAlgorithm ---------------------------------\n"
    "                                                                                         \n"
    "  Algorithm for estimating pxd gains (conversion factor from charge to ADU)              \n"
    " ----------------------------------------------------------------------------------------\n"
  );
}

CalibrationAlgorithm::EResult PXDGainCalibrationAlgorithm::calibrate()
{

  m_rootFile = new TFile("GainPlots.root", "recreate");
  m_rootFile->cd("");

  // Get counter histograms for MC and Data
  auto mc_counter = getObjectPtr<TH1I>("PXDMCCounter");
  auto data_counter = getObjectPtr<TH1I>("PXDDataCounter");

  // Initializing fit minimizer
  m_Minit2h = new TMinuit(1);
  m_Minit2h->SetFCN(FCNGain);
  double arglist[10];
  int ierflg = 0;
  arglist[0] = -1;
  //m_Minit2h->mnexcm("SET PRIntout", arglist, 1, ierflg);
  //m_Minit2h->mnexcm("SET NOWarnings", arglist, 0, ierflg);
  //arglist[0] = 1;
  //m_Minit2h->mnexcm("SET ERR", arglist, 1, ierflg);
  //arglist[0] = 0;
  //m_Minit2h->mnexcm("SET STRategy", arglist, 1, ierflg);
  //arglist[0] = 1;
  //m_Minit2h->mnexcm("SET GRAdient", arglist, 1, ierflg);
  //arglist[0] = 1e-6;
  //m_Minit2h->mnexcm("SET EPSmachine", arglist, 1, ierflg);

  // This is the PXD gain correction payload for conditions DB
  PXDGainMapPar* gainMapPar = new PXDGainMapPar();

  // Loop over all DCD-SWB pairs
  for (auto iPair = 1; iPair <= data_counter->GetXaxis()->GetNbins(); iPair++) {
    // The bin label is almost the name of the tree containing the calibration data
    string label = data_counter->GetXaxis()->GetBinLabel(iPair);

    // Parse label string format to read sensorID, DCD chipID and SWB chipID
    istringstream  stream(label);
    string token;
    getline(stream, token, '_');
    VxdID sensorID(token);

    getline(stream, token, '_');
    int iDCD = std::stoi(token);

    getline(stream, token, '_');
    int iSWB = std::stoi(token);

    // Read back the counters for number of collected clusters
    int numberOfDataHits = data_counter->GetBinContent(iPair);
    int numberOfMCHits = mc_counter->GetBinContent(iPair);

    // Only perform fitting, when enough data is available
    if (numberOfDataHits >= minClusters && numberOfMCHits >= minClusters) {

      // Get pointer to tree and setup all branch addresses
      string treename = "tree_" + label;
      m_tree = getObjectPtr<TTree>(treename);
      m_tree->SetBranchAddress("sensorID", &m_sensorID);
      m_tree->SetBranchAddress("uCellID", &m_uCellID);
      m_tree->SetBranchAddress("vCellID", &m_vCellID);
      m_tree->SetBranchAddress("signal", &m_signal);
      m_tree->SetBranchAddress("isMC", &m_isMC);

      //Calling optimized gain fit
      double gain, chi2;
      gain = 1;
      chi2 = -1;
      FitGain(gain, chi2);

      // Store gain
      gainMapPar->setGainCorrection(sensorID.getID(), iDCD * 6 + iSWB, gain);

      // Create some validation histos
      auto layer = sensorID.getLayerNumber();
      auto ladder = sensorID.getLadderNumber();
      auto sensor = sensorID.getSensorNumber();

      string histoname = str(format("signal_data_sensor_%1%_%2%_%3%_dcd_%4%_swb_%5%") % layer % ladder % sensor % iDCD % iSWB);
      TH1D dataHisto(histoname.c_str(), histoname.c_str(), N, fitRangeLower, fitRangeUpper);

      histoname = str(format("signal_mc_sensor_%1%_%2%_%3%_dcd_%4%_swb_%5%") % layer % ladder % sensor % iDCD % iSWB);
      TH1D mcHisto(histoname.c_str(), histoname.c_str(), N, fitRangeLower, fitRangeUpper);

      createValidationHistograms(dataHisto, mcHisto, gain);

      m_rootFile->cd();
      dataHisto.Write();
      mcHisto.Write();
    } else {
      if (numberOfMCHits < minClusters) {
        B2WARNING("Number of mc hits too small for fitting: " << numberOfMCHits << " < " << minClusters << ". Use default gain=1.0");
      }
      if (numberOfDataHits < minClusters) {
        B2WARNING("Number of data hits too small for fitting: " << numberOfDataHits << " < " << minClusters << ". Use default gain=1.0");
      }
    }
  }

  // Save the gain map to database. Note that this will set the database object name to the same as the collector but you
  // are free to change it.
  saveCalibration(gainMapPar, "PXDGainMapPar");

  // ROOT Output
  m_rootFile->Write();
  m_rootFile->Close();

  B2INFO("PXD Gain Calibration Successful");
  return c_OK;
}


void PXDGainCalibrationAlgorithm::FitGain(double& gain, double& amin)
{
  // Minuit parameters
  double arglist[10];
  int ierflg = 0;

  double gain0 = 1.0;
  double gainStep = 0.7;
  double gainUpperLimit = 5.0;
  double gainLowerLimit = 0.2;
  m_Minit2h->mnparm(0, "Gain",  gain0, gainStep, gainLowerLimit, gainUpperLimit, ierflg);

  // Perform fit
  arglist[0] = 100;
  arglist[1] = 1.;
  //m_Minit2h->mnexcm("MIGRAD", arglist, 2, ierflg);
  m_Minit2h->mnexcm("SIMPLEX", arglist, 2, ierflg);

  double edm, errdef;
  int nvpar, nparx, icstat;
  m_Minit2h->mnstat(amin, edm, errdef, nvpar, nparx, icstat);

  // Get fit results
  double ep;
  m_Minit2h->GetParameter(0, gain,  ep);
}


void PXDGainCalibrationAlgorithm::createValidationHistograms(TH1D& dataHist, TH1D& mcHist, float gain)
{
  // Loop over the tree is to fill the signal histograms
  const auto nEntries = m_tree->GetEntries();
  for (int i = 0; i < nEntries; ++i) {
    m_tree->GetEntry(i);

    double noise = gRandom->Gaus(0.0, noise_sigma);
    if (m_isMC) {
      mcHist.Fill(gain * m_signal + noise);
    } else {
      dataHist.Fill(m_signal + noise);
    }
  }
  return;
}


