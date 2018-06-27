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
#include <TFile.h>
#include <TH1D.h>

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
  int m_nBins = 20;

  /** Noise for smearing cluster charge */
  float m_noiseSigma = 3.0;

  /** Lower edge of fit range */
  float m_fitRangeLower = 25;

  /** Upper edge of fit range */
  float m_fitRangeUpper = 170;

  /** Signal array for data */
  vector<float> countData(m_nBins, 0.0);

  /** Signal array for mc */
  vector<float> countMC(m_nBins, 0.0);

  /** Function to minimize in minuit fit. (chi2) */
  void FCNGain(int&, double* grad, double& f, double* p, int)
  {
    double gain = p[0];
    B2DEBUG(99, "Current gain: " << gain << " current gradient " << grad[0]);

    TAxis signalAxis(m_nBins, m_fitRangeLower, m_fitRangeUpper);
    int sumData = 0;
    int sumMC = 0;
    std::fill(countData.begin(), countData.end(), 0);
    std::fill(countMC.begin(), countMC.end(), 0);

    // Loop over the tree is to fill the signal histograms
    const auto nEntries = m_tree->GetEntries();
    for (int i = 0; i < nEntries; ++i) {
      m_tree->GetEntry(i);

      double noise = gRandom->Gaus(0.0, m_noiseSigma);
      if (m_isMC) {
        int bin = signalAxis.FindFixBin(gain * m_signal + noise) - 1;
        if (bin < m_nBins and bin >= 0) {
          countMC[bin] += 1;
          sumMC += 1;
        }
      } else {
        int bin = signalAxis.FindFixBin(m_signal + noise) - 1;
        if (bin < m_nBins and bin >= 0) {
          countData[bin] += 1;
          sumData += 1;
        }
      }
    }

    double chi2 = 0;
    // Computing chi2
    for (int i = 0; i < m_nBins; ++i) {
      if (countData[i] > 0 && countMC[i] > 0) {
        float sigmaData = 2.0 * std::sqrt(countData[i]) / sumData;
        float sigmaMC = 2.0 * std::sqrt(countMC[i]) / sumMC;
        float sigma2 = sigmaData * sigmaData + sigmaMC * sigmaMC;
        chi2 += std::pow(countData[i] / sumData - countMC[i] / sumMC, 2) / sigma2;
      }
    }
    f = chi2;
  }

  /** Create validation histograms for Data and MC */
  void createValidationHistograms(int layerNumber, int ladderNumber, int sensorNumber, int iDCD, int iSWB, float gain,
                                  TFile* rootFile)
  {
    string histoname;

    histoname = str(format("signal_data_sensor_%1%_%2%_%3%_dcd_%4%_swb_%5%") % layerNumber % ladderNumber % sensorNumber % iDCD % iSWB);
    TH1D dataHist(histoname.c_str(), histoname.c_str(), m_nBins, m_fitRangeLower, m_fitRangeUpper);

    histoname = str(format("signal_mc_sensor_%1%_%2%_%3%_dcd_%4%_swb_%5%") % layerNumber % ladderNumber % sensorNumber % iDCD % iSWB);
    TH1D mcHist(histoname.c_str(), histoname.c_str(), m_nBins, m_fitRangeLower, m_fitRangeUpper);

    // Loop over the tree is to fill the signal histograms
    const auto nEntries = m_tree->GetEntries();
    for (int i = 0; i < nEntries; ++i) {
      m_tree->GetEntry(i);

      double noise = gRandom->Gaus(0.0, m_noiseSigma);
      if (m_isMC) {
        mcHist.Fill(gain * m_signal + noise);
      } else {
        dataHist.Fill(m_signal + noise);
      }
    }

    // Test that maximum bin is not at an edge of fit window ()
    auto dataMaxBin = dataHist.GetMaximumBin();
    auto mcMaxBin = mcHist.GetMaximumBin();

    if (dataMaxBin == 1 or dataMaxBin == m_nBins) {
      auto label = str(format("%1%_%2%_%3%_%4%_%5%") % layerNumber % ladderNumber % sensorNumber % iDCD % iSWB);
      B2WARNING(label << ": Maximum bin (" << dataMaxBin << ") in data at edge of fit window. Check fit result carefully.");
    }
    if (mcMaxBin == 1 or mcMaxBin == m_nBins) {
      auto label = str(format("%1%_%2%_%3%_%4%_%5%") % layerNumber % ladderNumber % sensorNumber % iDCD % iSWB);
      B2WARNING(label << ": Maximum bin (" << mcMaxBin << ") in mc at edge of fit window. Check fit result carefully.");
    }

    rootFile->cd();
    dataHist.Write();
    mcHist.Write();
    return;
  }
}


PXDGainCalibrationAlgorithm::PXDGainCalibrationAlgorithm():
  CalibrationAlgorithm("PXDGainCollector"),
  minClusters(1000), nBins(20), noiseSigma(3.0), fitRangeLower(25.0), fitRangeUpper(170)
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


  TFile* rootFile = new TFile("GainPlots.root", "recreate");
  rootFile->cd("");

  // Get counter histograms for MC and Data
  auto mc_counter = getObjectPtr<TH1I>("PXDMCCounter");
  auto data_counter = getObjectPtr<TH1I>("PXDDataCounter");

  // Initializing fit minimizer
  m_Minit2h = new TMinuit(1);
  m_Minit2h->SetFCN(FCNGain);
  double arglist[10];
  int ierflg = 0;
  arglist[0] = -1;
  m_Minit2h->mnexcm("SET PRIntout", arglist, 1, ierflg);
  m_Minit2h->mnexcm("SET NOWarnings", arglist, 0, ierflg);
  arglist[0] = 1;
  m_Minit2h->mnexcm("SET ERR", arglist, 1, ierflg);
  arglist[0] = 0;
  m_Minit2h->mnexcm("SET STRategy", arglist, 1, ierflg);
  //arglist[0] = 1;
  //m_Minit2h->mnexcm("SET GRAdient", arglist, 1, ierflg);
  //arglist[0] = 1e-6;
  //m_Minit2h->mnexcm("SET EPSmachine", arglist, 1, ierflg);

  // Override fitting parameter with user settings
  m_nBins = nBins;
  m_noiseSigma = noiseSigma;
  m_fitRangeLower = fitRangeLower;
  m_fitRangeUpper = fitRangeUpper;

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

      auto layerNumber = sensorID.getLayerNumber();
      auto ladderNumber = sensorID.getLadderNumber();
      auto sensorNumber = sensorID.getSensorNumber();
      string treename = str(format("tree_%1%_%2%_%3%_%4%_%5%") % layerNumber % ladderNumber % sensorNumber % iDCD % iSWB);
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

      // Create validation histos and do some tests
      createValidationHistograms(layerNumber, ladderNumber, sensorNumber, iDCD, iSWB, gain, rootFile);
    } else {
      if (numberOfMCHits < minClusters) {
        B2WARNING(label << ": Number of mc hits too small for fitting (" << numberOfMCHits << " < " << minClusters <<
                  "). Use default gain=1.0");
      }
      if (numberOfDataHits < minClusters) {
        B2WARNING(label << ": Number of data hits too small for fitting (" << numberOfDataHits << " < " << minClusters <<
                  "). Use default gain=1.0");
      }
    }
  }

  // Save the gain map to database. Note that this will set the database object name to the same as the collector but you
  // are free to change it.
  saveCalibration(gainMapPar, "PXDGainMapPar");

  // ROOT Output
  rootFile->Write();
  rootFile->Close();

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
  m_Minit2h->mnexcm("SIMPLEX", arglist, 2, ierflg);

  double edm, errdef;
  int nvpar, nparx, icstat;
  m_Minit2h->mnstat(amin, edm, errdef, nvpar, nparx, icstat);

  // Get fit results
  double ep;
  m_Minit2h->GetParameter(0, gain,  ep);
}

