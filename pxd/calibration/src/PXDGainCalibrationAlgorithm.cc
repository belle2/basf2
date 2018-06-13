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

#include <pxd/unpacking/PXDMappingLookup.h>

#include <string>
#include <tuple>
#include <numeric>
#include <algorithm>
#include <functional>

#include <boost/format.hpp>
#include <cmath>


//ROOT
#include <TMath.h>
#include <TAxis.h>
#include <TMinuit.h>

using namespace std;
using boost::format;
using namespace Belle2;


// Anonymous namespace for data objects used by both ECLWaveformFitModule class and FCN2h function for MINUIT minimization.
namespace {

  //Signal array for data
  double FitData[128];

  //Signal array for mc
  double FitMC[128];

  //Function to minimize in minuit fit. (chi2)
  void FCN2h(int&, double* grad, double& f, double* p, int)
  {
    constexpr int N = 128;
    double df[N], da[N];
    const double gain = p[0];
    double chi2 = 0;
    const double ErrorPoint = 0.01777777777;

    //computing difference between current fit result and adc data array
    for (int i = 0; i < N; ++i) df[i] = FitData[i] - FitMC[i];

    //computing chi2.  Error set to +/- 7.5 adc units (identity matrix)
    for (int i = 0; i < N; ++i) da[i] = ErrorPoint * df[i];
    for (int i = 0; i < N; ++i) {
      chi2 += da[i] * df[i];
    }
    f = chi2;
  }

}



PXDGainCalibrationAlgorithm::PXDGainCalibrationAlgorithm():
  CalibrationAlgorithm("PXDGainCollector"),
  minDigits(50)
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

  string treename = string("tree");
  auto tree = getObjectPtr<TTree>(treename);

  const auto nEntries = tree->GetEntries();
  B2INFO("Number of clusters is " << nEntries);

  tree->SetBranchAddress("sensorID", &m_sensorID);
  tree->SetBranchAddress("uCellID", &m_uCellID);
  tree->SetBranchAddress("vCellID", &m_vCellID);
  tree->SetBranchAddress("signal", &m_signal);
  tree->SetBranchAddress("isMC", &m_isMC);

  int nDCD = 4;
  int nSWB = 6;

  // List of sensors found in data
  vector< pair<VxdID, int > > sensorList;

  for (int i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);

    if (not m_isMC) {
      auto sensorID = VxdID(m_sensorID);
      auto it = std::find_if(sensorList.begin(), sensorList.end(),
      [&](const pair<VxdID, int>& element) { return element.first == sensorID ;});

      //Sensor exists in vector
      if (it != sensorList.end()) {
        //increment key in map
        it->second++;
      }
      //Sensor name does not exist
      else {
        //Not found, insert in vector
        sensorList.push_back(pair<VxdID, int>(VxdID(m_sensorID), 1));
      }
    }
  }

  // Loop over sensorList to select sensors for
  // next calibration step

  // Initializing fit minimizer
  m_Minit2h = new TMinuit(1);
  m_Minit2h->SetFCN(FCN2h);
  double arglist[10];
  int ierflg = 0;
  arglist[0] = -1;
  m_Minit2h->mnexcm("SET PRIntout", arglist, 1, ierflg);
  m_Minit2h->mnexcm("SET NOWarnings", arglist, 0, ierflg);
  arglist[0] = 1;
  m_Minit2h->mnexcm("SET ERR", arglist, 1, ierflg);
  arglist[0] = 0;
  m_Minit2h->mnexcm("SET STRategy", arglist, 1, ierflg);
  arglist[0] = 1;
  m_Minit2h->mnexcm("SET GRAdient", arglist, 1, ierflg);
  arglist[0] = 1e-6;
  m_Minit2h->mnexcm("SET EPSmachine", arglist, 1, ierflg);

  for (auto iter : sensorList) {
    auto sensorID = iter.first;
    auto counter = iter.second;

    for (int areaID = 0; areaID < 1 /*nDCD * nSWB*/; areaID++) {
      int iDCD = areaID / 6 + 1;
      int iSWB = areaID % 6 + 1;

      //Calling optimized fit
      double gain, chi2;
      chi2 = -1;
      FitGain(gain, chi2);

      auto residual = computeResidual(1.0, treename, VxdID(sensorID), areaID);
      B2INFO("Residual is " << std::to_string(residual));

      string histoname = str(format("signal_data_sensor_%1%_%2%_%3%_dcd_%4%_swb_%5%") % sensorID.getLayerNumber() %
                             sensorID.getLadderNumber() % sensorID.getSensorNumber() % iDCD % iSWB);
      TH1D dataHisto(histoname.c_str(), histoname.c_str(), 128, 0, 255);
      dataHisto.SetXTitle("cluster charge / ADU");
      dataHisto.SetYTitle("number of clusters");

      histoname = str(format("signal_mc_sensor_%1%_%2%_%3%_dcd_%4%_swb_%5%") % sensorID.getLayerNumber() % sensorID.getLadderNumber() %
                      sensorID.getSensorNumber() % iDCD % iSWB);
      TH1D mcHisto(histoname.c_str(), histoname.c_str(), 128, 0, 255);
      mcHisto.SetXTitle("cluster charge / ADU");
      mcHisto.SetYTitle("number of clusters");

      createValidationHistograms(dataHisto, mcHisto, 1.0, treename, VxdID(sensorID), areaID);

      m_rootFile->cd();
      dataHisto.Write();
      mcHisto.Write();
    }

  }

  // Save the cluster positions to database.
  //saveCalibration(positionEstimator, "PXDClusterPositionEstimatorPar");

  //storing fit results
  //aECLDsp.setTwoComponentTotalAmp(p2_a + p2_a1);
  //aECLDsp.setTwoComponentHadronAmp(p2_a1);
  //aECLDsp.setTwoComponentChi2(p2_chi2);
  //aECLDsp.setTwoComponentTime(p2_t);
  //aECLDsp.setTwoComponentBaseline(p2_b);

  // ROOT Output
  m_rootFile->Write();
  m_rootFile->Close();

  B2INFO("PXD Gain Calibration Successful");
  return c_OK;
}

float PXDGainCalibrationAlgorithm::computeResidual(float gain, const std::string& treename, const VxdID& sensorID, int areaID)
{

  auto tree = getObjectPtr<TTree>(treename);

  tree->SetBranchAddress("sensorID", &m_sensorID);
  tree->SetBranchAddress("uCellID", &m_uCellID);
  tree->SetBranchAddress("vCellID", &m_vCellID);
  tree->SetBranchAddress("signal", &m_signal);
  tree->SetBranchAddress("isMC", &m_isMC);

  // Use TAxis to bin the data
  int nBins = 128;
  TAxis signalAxis(nBins, 0, 255);

  // Some counters for Data and MC
  vector<float> countData(nBins, 0.0);
  vector<float> countMC(nBins, 0.0);
  int sumData = 0;
  int sumMC = 0;

  // Loop over the tree is to fill the signal histograms
  const auto nEntries = tree->GetEntries();
  for (int i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);

    int nDCD = 4;
    int nSWB = 6;

    auto currentSensorID = VxdID(m_sensorID);
    int iDCD = PXD::PXDMappingLookup::getDCDID(m_uCellID, m_vCellID, sensorID) - 1;
    int iSWB = PXD::PXDMappingLookup::getSWBID(m_vCellID) - 1;
    int currentAreaID = iDCD * nSWB + iSWB;

    if (currentSensorID == sensorID and currentAreaID == areaID) {

      if (m_isMC) {
        int bin = signalAxis.FindFixBin(gain * m_signal) - 1;
        if (bin < nBins) {
          countMC[bin] += 1;
          sumMC += 1;
        }
      } else {
        int bin = signalAxis.FindFixBin(m_signal) - 1;
        if (bin < nBins) {
          countData[bin] += 1;
          sumData += 1;
        }
      }
    }
  }
  float residual = 0;
  for (int i = 0; i < nBins; i++) {
    residual += std::pow(countData[i] / sumData  - countMC[i] / sumMC, 2);
  }
  return residual;
}



void PXDGainCalibrationAlgorithm::createValidationHistograms(TH1D& dataHist, TH1D& mcHist, float gain, const std::string& treename,
    const VxdID& sensorID, int areaID)
{

  auto tree = getObjectPtr<TTree>(treename);

  tree->SetBranchAddress("sensorID", &m_sensorID);
  tree->SetBranchAddress("uCellID", &m_uCellID);
  tree->SetBranchAddress("vCellID", &m_vCellID);
  tree->SetBranchAddress("signal", &m_signal);
  tree->SetBranchAddress("isMC", &m_isMC);

  // Loop over the tree is to fill the signal histograms
  const auto nEntries = tree->GetEntries();
  for (int i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);

    int nDCD = 4;
    int nSWB = 6;

    auto currentSensorID = VxdID(m_sensorID);
    int iDCD = PXD::PXDMappingLookup::getDCDID(m_uCellID, m_vCellID, sensorID) - 1;
    int iSWB = PXD::PXDMappingLookup::getSWBID(m_vCellID) - 1;
    int currentAreaID = iDCD * nSWB + iSWB;

    if (currentSensorID == sensorID and currentAreaID == areaID) {

      if (m_isMC) {
        mcHist.Fill(gain * m_signal);
      } else {
        dataHist.Fill(m_signal);
      }
    }
  }
  return;
}

// Optimized fit
void PXDGainCalibrationAlgorithm::FitGain(double& gain, double& amin)
{
  // Minuit parameters
  double arglist[10];
  int ierflg = 0;

  double gain0 = 1.0;
  double gainStep = 0.001;
  double gainUpperLimit = 2.0;
  double gainLowerLimit = 0.5;
  m_Minit2h->mnparm(0, "Gain",  gain0, gainStep, gainLowerLimit, gainUpperLimit, ierflg);

  // Perform fit
  arglist[0] = 500;
  arglist[1] = 1.;
  m_Minit2h->mnexcm("MIGRAD", arglist, 2, ierflg);

  double edm, errdef;
  int nvpar, nparx, icstat;
  m_Minit2h->mnstat(amin, edm, errdef, nvpar, nparx, icstat);

  // Get fit results
  double ep;
  m_Minit2h->GetParameter(0, gain,  ep);
}
