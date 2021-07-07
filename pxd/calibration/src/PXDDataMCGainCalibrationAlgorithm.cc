/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/calibration/PXDDataMCGainCalibrationAlgorithm.h>
#include <pxd/dbobjects/PXDClusterChargeMapPar.h>
#include <pxd/dbobjects/PXDGainMapPar.h>

#include <string>
#include <algorithm>
#include <map>
#include <set>

#include <sstream>
#include <iostream>

#include <boost/format.hpp>
#include <cmath>


//ROOT
#include <TRandom.h>
#include <TF1.h>
#include <TH2I.h>
#include <TCanvas.h>

using namespace std;
using boost::format;
using namespace Belle2;


// Anonymous namespace for data objects used by PXDDataMCGainCalibrationAlgorithm class
namespace {

  /** Signal in ADU of collected clusters */
  int m_signal;
  /** Run number to be stored in dbtree */
  int m_run;
  /** Experiment number to be stored in dbtree */
  int m_exp;

  /** Helper function to extract number of bins along u side and v side from counter histogram labels. */
  void getNumberOfBins(const std::shared_ptr<TH1I>& histo_ptr, unsigned short& nBinsU, unsigned short& nBinsV)
  {
    set<unsigned short> uBinSet;
    set<unsigned short> vBinSet;

    // Loop over all bins of input histo
    for (auto histoBin = 1; histoBin <= histo_ptr->GetNbinsX(); histoBin++) {
      // The bin label contains the vxdid, uBin and vBin
      string label = histo_ptr->GetXaxis()->GetBinLabel(histoBin);

      // Parse label string format to read sensorID, uBin and vBin
      istringstream  stream(label);
      string token;
      getline(stream, token, '_');
      getline(stream, token, '_');
      unsigned short uBin = std::stoi(token);

      getline(stream, token, '_');
      unsigned short vBin = std::stoi(token);

      uBinSet.insert(uBin);
      vBinSet.insert(vBin);
    }

    if (uBinSet.empty() || vBinSet.empty()) {
      B2FATAL("Not able to determine the grid size. Something is wrong with collected data.");
    } else {
      nBinsU = *uBinSet.rbegin() + 1;
      nBinsV = *vBinSet.rbegin() + 1;
    }
  }

  /** Helper function to extract number of sensors from counter histogram labels. */
  unsigned short getNumberOfSensors(const std::shared_ptr<TH1I>& histo_ptr)
  {
    set<unsigned short> sensorSet;

    // Loop over all bins of input histo
    for (auto histoBin = 1; histoBin <= histo_ptr->GetNbinsX(); histoBin++) {
      // The bin label contains the vxdid, uBin and vBin
      string label = histo_ptr->GetXaxis()->GetBinLabel(histoBin);

      // Parse label string format to read sensorID, uBin and vBin
      istringstream  stream(label);
      string token;
      getline(stream, token, '_');
      VxdID sensorID(token);
      sensorSet.insert(sensorID.getID());
    }

    return sensorSet.size();
  }

}


PXDDataMCGainCalibrationAlgorithm::PXDDataMCGainCalibrationAlgorithm():
  CalibrationAlgorithm("PXDClusterChargeCollector"),
  minClusters(5000), noiseSigma(0.6), safetyFactor(2.0), forceContinue(false), strategy(0),
  doCalibration(false), useChargeHistogram(false), chargePayloadName("PXDClusterChargeMapPar")
{
  setDescription(
    " ------------------------------ PXDDataMCGainCalibrationAlgorithm -----------------------------------\n"
    "                                                                                               \n"
    "  Algorithm for estimating pxd median/MPV cluster charges for different position on sensor in ADU, and optionally calculate gain \n"
    " ------------------------------------------------------------------------------------------------\n"
  );

}


CalibrationAlgorithm::EResult PXDDataMCGainCalibrationAlgorithm::calibrate()
{

  // Get counter histogram
  auto cluster_counter = getObjectPtr<TH1I>("PXDClusterCounter");
  if (!cluster_counter) {
    B2INFO("Not enough Data: cluster counter does not exist ");
    return c_NotEnoughData;
  }

  // Extract number of sensors from counter histograms
  auto nSensors = getNumberOfSensors(cluster_counter);

  // Extract the number of grid bins from counter histograms
  unsigned short nBinsU = 0;
  unsigned short nBinsV = 0;
  getNumberOfBins(cluster_counter, nBinsU, nBinsV);

  // Check that we have collected enough Data
  if (cluster_counter->GetEntries() < int(safetyFactor * minClusters * nSensors * nBinsU * nBinsV)) {
    if (not forceContinue) {
      B2INFO("Not enough Data: Only " <<  cluster_counter->GetEntries() << " hits were collected but " << int(safetyFactor * minClusters *
             nSensors * nBinsU * nBinsV) << " needed!");
      return c_NotEnoughData;
    } else {
      B2INFO("Continue despite low statistics: Only " <<  cluster_counter->GetEntries() << " hits were collected but" << int(
               safetyFactor * minClusters *
               nSensors * nBinsU * nBinsV) << " would be desirable!");
    }
  }

  B2INFO("Start calibration using a " << nBinsU << "x" << nBinsV << " grid per sensor.");

  // This is the PXD charge calibration payload for conditions DB
  PXDClusterChargeMapPar* chargeMapPar = new PXDClusterChargeMapPar(nBinsU, nBinsV);
  // This is the PXD gain correction payload for conditions DB
  PXDGainMapPar* gainMapPar = new PXDGainMapPar(nBinsU, nBinsV);
  set<VxdID> pxdSensors;

  // Read back existing DB payloads
  PXDClusterChargeMapPar* chargeMapPtr = nullptr;
  PXDGainMapPar* gainMapPtr = nullptr;
  auto dbtree = getObjectPtr<TTree>("dbtree");
  dbtree->SetBranchAddress("run", &m_run);
  dbtree->SetBranchAddress("exp", &m_exp);
  dbtree->SetBranchAddress("chargeMap", &chargeMapPtr);
  dbtree->SetBranchAddress("gainMap", &gainMapPtr);

  // Loop over all bins of input histo
  for (auto histoBin = 1; histoBin <= cluster_counter->GetNbinsX(); histoBin++) {

    // The bin label contains the vxdid, uBin and vBin
    string label = cluster_counter->GetXaxis()->GetBinLabel(histoBin);

    // Parse label string format to read sensorID, uBin and vBin
    istringstream  stream(label);
    string token;
    getline(stream, token, '_');
    VxdID sensorID(token);

    getline(stream, token, '_');
    unsigned short uBin = std::stoi(token);

    getline(stream, token, '_');
    unsigned short vBin = std::stoi(token);

    // Read back the counters for number of collected clusters
    int numberOfDataHits = cluster_counter->GetBinContent(histoBin);

    // Only perform fitting, when enough data is available
    if (numberOfDataHits >= minClusters) {

      B2INFO("start EstimateCharge");
      // Compute the cluster charge or gain for the part of PXD
      auto Charge = EstimateCharge(sensorID, uBin, vBin, histoBin);

      // Store the charge or gain
      if (!doCalibration) {

        B2INFO("EstimateCharge: sensor " << sensorID.getID() <<  " U " << uBin << " V " << vBin << " Charge " << Charge);
        chargeMapPar->setContent(sensorID.getID(), uBin, vBin, Charge);

      } else {

        auto Gain = 0.0;

        if (Charge <= 0.0) {
          B2WARNING("Retrieved negative charge for data for sensor=" << sensorID << " uBin=" << uBin << " vBin=" << vBin <<
                    ". Set gain to default value (=1.0).");
          Gain = 1.0;
        } else {
          dbtree->GetEntry(0);
          double mcCharge = chargeMapPtr->getContent(sensorID.getID(), uBin, vBin);
          //GetChargeFromDB(sensorID, uBin, vBin, dbtree);
          if (mcCharge <= 0.0) {
            B2WARNING("Retrieved negative charge for MC from DB for sensor=" << sensorID << " uBin=" << uBin << " vBin=" << vBin <<
                      ". Set gain to default value (=1.0).");
            Gain = 1.0;
          } else {
            Gain = Charge / mcCharge;
            B2INFO("Estimated Gain: sensor " << sensorID.getID() <<  " U " << uBin << " V " << vBin << " Gain " << Gain << " = " << Charge <<
                   " / " << mcCharge);
          }

        }

        gainMapPar->setContent(sensorID.getID(), uBin, vBin, Gain);

      }

    } else {

      B2WARNING(label << ": Number of data hits too small for fitting (" << numberOfDataHits << " < " << minClusters <<
                "). Use default value of 0 for charge, 1 for gain.");
      if (!doCalibration) chargeMapPar->setContent(sensorID.getID(), uBin, vBin, 0.0);
      else                gainMapPar->setContent(sensorID.getID(), uBin, vBin, 1.0);

    }

    pxdSensors.insert(sensorID);

  }

  chargeMapPtr = nullptr;
  gainMapPtr = nullptr;

  // Save the charge map to database. Note that this will set the database object name to the same as the collector but you
  // are free to change it.
  if (!doCalibration) {
    saveCalibration(chargeMapPar, chargePayloadName);

    B2INFO("PXD Cluster Charge Calibration Successful");
    return c_OK;

  } else {

    // Post processing of gain map. It is possible that the gain
    // computation failed on some parts. Here, we replace default
    // values (1.0) by local averages of neighboring sensor parts.

    for (const auto& sensorID : pxdSensors) {
      for (unsigned short vBin = 0; vBin < nBinsV; ++vBin) {
        float meanGain = 0;
        unsigned short nGood = 0;
        unsigned short nBad = 0;
        for (unsigned short uBin = 0; uBin < nBinsU; ++uBin) {
          auto gain = gainMapPar->getContent(sensorID.getID(), uBin, vBin);
          // Filter default gains
          if (gain != 1.0) {
            nGood += 1;
            meanGain += gain;
          } else {
            nBad += 1;
          }
        }
        B2RESULT("Gain calibration on sensor=" << sensorID << " and vBin=" << vBin << " was successful on " << nGood << "/" << nBinsU <<
                 " uBins.");

        // Check if we can repair bad calibrations with a local avarage
        if (nGood > 0 && nBad > 0) {
          meanGain /= nGood;
          for (unsigned short uBin = 0; uBin < nBinsU; ++uBin) {
            auto gain = gainMapPar->getContent(sensorID.getID(), uBin, vBin);
            if (gain == 1.0) {
              gainMapPar->setContent(sensorID.getID(), uBin, vBin, meanGain);
              B2RESULT("Gain calibration on sensor=" << sensorID << ", vBin=" << vBin << " uBin " << uBin << ": Replace default gain wih average "
                       << meanGain);
            }
          }
        }
      }
    }

    // Save the gain map to database. Note that this will set the database object name to the same as the collector but you
    // are free to change it.
    saveCalibration(gainMapPar, "PXDGainMapPar");

    B2INFO("PXD Gain Calibration Successful");
    return c_OK;

  }

}


double PXDDataMCGainCalibrationAlgorithm::EstimateCharge(VxdID sensorID, unsigned short uBin, unsigned short vBin,
                                                         unsigned short histoBin)
{

  // Construct a tree name for requested part of PXD
  auto layerNumber = sensorID.getLayerNumber();
  auto ladderNumber = sensorID.getLadderNumber();
  auto sensorNumber = sensorID.getSensorNumber();

  if (!useChargeHistogram) {
    const string treename = str(format("tree_%1%_%2%_%3%_%4%_%5%") % layerNumber % ladderNumber % sensorNumber % uBin % vBin);
    // Vector with cluster signals from collected data
    vector<double> signals;
    // Fill data_signal vector from input data
    auto tree = getObjectPtr<TTree>(treename);
    tree->SetBranchAddress("signal", &m_signal);

    // Loop over tree
    const auto nEntries = tree->GetEntries();
    int incr(1);
    if (int(nEntries / minClusters) > 2) incr = int(2 * nEntries / minClusters);
    for (int i = 0; i < nEntries; i += incr) {
      tree->GetEntry(i);
      double noise = gRandom->Gaus(0.0, noiseSigma);
      signals.push_back(m_signal + noise); //qyliu: why we introduce noise simulation here?
    }


    if (strategy == 0) {
      double median = CalculateMedian(signals);
      B2INFO("EstimateCharge: sensor " << sensorID.getID() << "(" << layerNumber << "," << ladderNumber << "," << sensorNumber
             <<  ") U " << uBin << " V " << vBin
             << " Charge " << median);
      return median;
    } else if (strategy == 1) {
      double median = CalculateMedian(signals);
      double landaumpv  = FitLandau(signals);
      double diff  = (landaumpv - median);
      double difff = 0.;
      if (landaumpv > 0.) difff = (landaumpv - median) / landaumpv;
      B2INFO("EstimateCharge: sensor " << sensorID.getID() << "(" << layerNumber << "," << ladderNumber << "," << sensorNumber
             <<  ") U " << uBin << " V " << vBin
             << " Charge " << landaumpv << " Median " << median << " diff = " << diff << "/" << difff);
      return landaumpv; //FitLandau(signals);
    } else if (strategy == 2) {
      double mean = 0;
      for (auto& each : signals)
        mean += each;
      if (signals.size() > 0) mean = mean / signals.size();
      return mean;
    } else {
      B2FATAL("strategy unavailable, use 0 for medians, 1 for landau fit and 2 for mean!");
    }

  } else {

    auto cluster_counter = getObjectPtr<TH2I>("PXDClusterCharge");
    TH1D* hist_signals = cluster_counter->ProjectionY("proj", histoBin, histoBin);

    if (strategy == 0) {
      double median = CalculateMedian(hist_signals);
      B2INFO("EstimateCharge: sensor " << sensorID.getID() << "(" << layerNumber << "," << ladderNumber << "," << sensorNumber
             <<  ") U " << uBin << " V " << vBin
             << " Charge " << median);
      return median;
    }
    if (strategy == 1) {
      double median = CalculateMedian(hist_signals);
      double landaumpv  = FitLandau(hist_signals);
      double diff  = (landaumpv - median);
      double difff = 0.;
      if (landaumpv > 0.) difff = (landaumpv - median) / landaumpv;
      B2INFO("EstimateCharge: sensor " << sensorID.getID() << "(" << layerNumber << "," << ladderNumber << "," << sensorNumber
             <<  ") U " << uBin << " V " << vBin
             << " Charge " << landaumpv << " Median " << median << " diff = " << diff << "/" << difff);
      return landaumpv; //FitLandau(signals);
    } else if (strategy == 2) {
      return hist_signals->GetMean();
    } else {
      B2FATAL("strategy unavailable, use 0 for medians, 1 for landau fit and 2 for mean!");
    }

    delete hist_signals;

  }

}

double PXDDataMCGainCalibrationAlgorithm::CalculateMedian(vector<double>& signals)
{
  auto size = signals.size();

  if (size == 0) {
    return 0.0;  // Undefined, really.
  } else {
    sort(signals.begin(), signals.end());
    if (size % 2 == 0) {
      return (signals[size / 2 - 1] + signals[size / 2]) / 2;
    } else {
      return signals[size / 2];
    }
  }
}

double PXDDataMCGainCalibrationAlgorithm::CalculateMedian(TH1D* hist_signals)
{
  auto size = hist_signals->GetEntries();

  if (size == 0)  return 0.0;  // Undefined.

  int sum = 0;
  for (int ibin = 0; ibin < hist_signals->GetNbinsX(); ++ibin) {
    sum += hist_signals->GetBinContent(ibin + 1);
    if (sum > size / 2) {
      return hist_signals->GetBinLowEdge(ibin + 1);
    }
  }

  B2WARNING("Could not find median! using default value 0.0!");
  return 0.0;
}

double PXDDataMCGainCalibrationAlgorithm::FitLandau(vector<double>& signals)
{
  auto size = signals.size();
  if (size == 0) return 0.0; // Undefined, really.

  // get max and min values of vector
  int max = *max_element(signals.begin(), signals.end());
  int min = *min_element(signals.begin(), signals.end());
  // make even bins
  if ((max - min) % 2) max--;
  int nbin = max - min;

  // create histogram to hold signals and fill it
  TH1D* hist_signals = new TH1D("", "", nbin, min, max);
  for (auto it = signals.begin(); it != signals.end(); ++it) {
    hist_signals->Fill(*it);
  }

  // create fit function
  TF1* landau = new TF1("landau", "TMath::Landau(x,[0],[1])*[2]", min, max);
  landau->SetParNames("MPV", "sigma", "scale");
  landau->SetParameters(35, 8, 1000);
  landau->SetParLimits(0, 0., 80.);

  //do fit and get results, fit range restricted to exclude low charge peak
  float fitmin(min);
  float fitmax(350);
  Int_t status = hist_signals->Fit("landau", "Lq", "", fitmin, fitmax);
  double MPV = landau->GetParameter("MPV");

  B2INFO("Fit result: " << status << " MPV " <<  MPV << " sigma " << landau->GetParameter("sigma")
         << " scale " << landau->GetParameter("scale") << " chi2 " << landau->GetChisquare());

  // clean up
  delete hist_signals;
  delete landau;

  // check fit status
  if (status == 0) return MPV;
  else {
    B2WARNING("Fit failed! using default value 0.0!");
    return 0.0;
  }
}

double PXDDataMCGainCalibrationAlgorithm::FitLandau(TH1D* hist_signals)
{
  auto size = hist_signals->GetEntries();
  if (size == 0) return 0.0; // Undefined.

  int max = hist_signals->GetBinLowEdge(hist_signals->GetNbinsX() + 1);
  int min = hist_signals->GetBinLowEdge(1);

  // create fit function
  TF1* landau = new TF1("landau", "TMath::Landau(x,[0],[1])*[2]", min, max);
  landau->SetParNames("MPV", "sigma", "scale");
  landau->SetParameters(35, 8, 1000);
  landau->SetParLimits(0, 0., 80.);

  // do fit and get results, fit range restricted to exclude low charge peak
  float fitmin(min);
  float fitmax(250);
  Int_t status = hist_signals->Fit("landau", "Lq", "", fitmin, fitmax);
  double MPV = landau->GetParameter("MPV");

  B2INFO("Fit result: " << status << " MPV " <<  MPV << " sigma " << landau->GetParameter("sigma")
         << " scale " << landau->GetParameter("scale") << " chi2 " << landau->GetChisquare());

  // clean up
  delete landau;

  // check fit status
  if (status == 0) return MPV;
  else {
    B2WARNING("Fit failed! using default value 0.0!");
    return 0.0;
  }
}

