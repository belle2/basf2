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
#include <map>

#include <boost/format.hpp>
#include <cmath>


//ROOT
#include <TMath.h>
#include <TAxis.h>
#include <TMinuit.h>
#include <TCanvas.h>
#include <TRandom.h>

using namespace std;
using boost::format;
using namespace Belle2;


// Anonymous namespace for data objects used by both PXDGainCalibrationAlgorithm class and FCN2h function for MINUIT minimization.
namespace {

  constexpr int nDCD = 4;
  constexpr int nSWB = 6;

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

  // Number of bins for cluster charge
  constexpr int N = 64;

  // Current sensorID
  int m_currentSensorID = -1;

  // Current area ID
  int m_currentAreaID = -1;

  // Signal array for data
  vector<float> countData(N, 0.0);

  // Signal array for mc
  vector<float> countMC(N, 0.0);

  // Function to minimize in minuit fit. (chi2)
  void FCN2h(int&, double* grad, double& f, double* p, int)
  {

    double gain = p[0];

    B2INFO("Gain is " << std::to_string(gain));

    TAxis signalAxis(N, 0, 255);
    int sumData = 0;
    int sumMC = 0;
    std::fill(countData.begin(), countData.end(), 0);
    std::fill(countMC.begin(), countMC.end(), 0);

    // Loop over the tree is to fill the signal histograms
    const auto nEntries = m_tree->GetEntries();
    for (int i = 0; i < nEntries; ++i) {
      m_tree->GetEntry(i);

      auto sensorID = VxdID(m_sensorID);
      int iDCD = PXD::PXDMappingLookup::getDCDID(m_uCellID, m_vCellID, sensorID) - 1;
      int iSWB = PXD::PXDMappingLookup::getSWBID(m_vCellID) - 1;
      int areaID = iDCD * nSWB + iSWB;

      if (m_currentSensorID == m_sensorID and m_currentAreaID == areaID) {

        if (m_isMC) {
          double noise = gRandom->Gaus(0.0, 0.7);
          int bin = signalAxis.FindFixBin(gain * m_signal + noise) - 1;
          if (bin < N) {
            countMC[bin] += 1;
            sumMC += 1;
          }
        } else {
          int bin = signalAxis.FindFixBin(m_signal) - 1;
          if (bin < N) {
            countData[bin] += 1;
            sumData += 1;
          }
        }
      }
    }

    double chi2 = 0;
    // Computing chi2.  Error set to +/- XY adc units (identity matrix)
    for (int i = 0; i < N; ++i) {
      chi2 += std::pow(countData[i] / sumData - countMC[i] / sumMC, 2);
    }

    f = chi2;
    B2INFO("Chi2 is " << std::to_string(chi2));
  }

}


PXDGainCalibrationAlgorithm::PXDGainCalibrationAlgorithm():
  CalibrationAlgorithm("PXDGainCollector"),
  minClusters(1000)
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

  m_tree = getObjectPtr<TTree>("tree");
  m_tree->SetBranchAddress("sensorID", &m_sensorID);
  m_tree->SetBranchAddress("uCellID", &m_uCellID);
  m_tree->SetBranchAddress("vCellID", &m_vCellID);
  m_tree->SetBranchAddress("signal", &m_signal);
  m_tree->SetBranchAddress("isMC", &m_isMC);

  const auto nEntries = m_tree->GetEntries();

  // List of sensors found in data
  vector< pair<VxdID, int > > sensorList;

  for (int i = 0; i < nEntries; ++i) {
    m_tree->GetEntry(i);

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




  map<VxdID, float> myGains;
  myGains[VxdID("1.1.1")] = 1.8;
  myGains[VxdID("1.1.2")] = 1.4;
  myGains[VxdID("2.1.1")] = 1.9;
  myGains[VxdID("2.1.2")] = 1.5;


  for (auto iter : sensorList) {
    auto sensorID = iter.first;
    auto counter = iter.second;

    for (int areaID = 0; areaID < 1 /*nDCD * nSWB*/; areaID++) {
      int iDCD = areaID / 6 + 1;
      int iSWB = areaID % 6 + 1;

      m_currentSensorID = sensorID.getID();
      m_currentAreaID = areaID;

      //Calling optimized fit
      double gain, chi2;
      gain = 1;
      chi2 = -1;
      //FitGain(gain, chi2);

      B2INFO("Fitted gain is " << std::to_string(gain));


      auto layer = sensorID.getLayerNumber();
      auto ladder = sensorID.getLadderNumber();
      auto sensor = sensorID.getSensorNumber();

      string histoname = str(format("signal_data_sensor_%1%_%2%_%3%_dcd_%4%_swb_%5%") % layer % ladder % sensor % iDCD % iSWB);
      TH1D dataHisto(histoname.c_str(), histoname.c_str(), 128, 0, 255);
      dataHisto.SetXTitle("cluster charge / ADU");
      dataHisto.SetLineColor(kBlack);
      dataHisto.SetYTitle("number of clusters");

      histoname = str(format("signal_mc_sensor_%1%_%2%_%3%_dcd_%4%_swb_%5%") % layer % ladder % sensor % iDCD % iSWB);
      TH1D mcHisto(histoname.c_str(), histoname.c_str(), 128, 0, 255);
      mcHisto.SetXTitle("cluster charge / ADU");
      mcHisto.SetLineColor(kBlue);
      mcHisto.SetYTitle("number of clusters");


      createValidationHistograms(dataHisto, mcHisto, myGains[sensorID]);

      m_rootFile->cd();
      dataHisto.Write();
      mcHisto.Write();


      TCanvas canvas(str(format("sensor_%1%_%2%_%3%_dcd_%4%_swb_%5%") % layer % ladder % sensor % iDCD % iSWB).c_str());

      mcHisto.DrawNormalized();
      dataHisto.DrawNormalized("same");
      canvas.Update();
      canvas.Write();
    }
  }

  // Save the cluster positions to database.
  //saveCalibration(positionEstimator, "PXDClusterPositionEstimatorPar");

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
  double gainStep = 10.1;
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





void PXDGainCalibrationAlgorithm::createValidationHistograms(TH1D& dataHist, TH1D& mcHist, float gain)
{

  // Loop over the tree is to fill the signal histograms
  const auto nEntries = m_tree->GetEntries();
  for (int i = 0; i < nEntries; ++i) {
    m_tree->GetEntry(i);

    auto sensorID = VxdID(m_sensorID);
    int iDCD = PXD::PXDMappingLookup::getDCDID(m_uCellID, m_vCellID, sensorID) - 1;
    int iSWB = PXD::PXDMappingLookup::getSWBID(m_vCellID) - 1;
    int areaID = iDCD * nSWB + iSWB;

    if (m_currentSensorID == m_sensorID and m_currentAreaID == areaID) {

      if (m_isMC) {
        double noise = gRandom->Gaus(0.0, 0.7);
        mcHist.Fill(gain * m_signal + noise);
      } else {
        dataHist.Fill(m_signal);
      }
    }
  }
  return;
}


