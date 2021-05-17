/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Qingyuan Liu                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <pxd/calibration/PXDCalibrationUtilities.h>
#include <vxd/dataobjects/VxdID.h>

#include <string>
#include <algorithm>
#include <set>

#include <sstream>
#include <iostream>

#include <boost/format.hpp>

#include <TF1.h>

using namespace std;
using boost::format;

namespace Belle2 {
  namespace PXD {

    /** Helper function to extract number of bins along u side and v side from counter histogram labels. */
    void getNumberOfBins(const std::shared_ptr<TH1I>& histo_ptr, unsigned short& nBinsU, unsigned short& nBinsV)
    {
      set<unsigned short> uBinSet;
      set<unsigned short> vBinSet;

      // Loop over all bins of input histo
      for (auto histoBin = 1; histoBin <= histo_ptr->GetXaxis()->GetNbins(); histoBin++) {
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
      for (auto histoBin = 1; histoBin <= histo_ptr->GetXaxis()->GetNbins(); histoBin++) {
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

    /** Helper function to calculate a median from unsorted signal vector. The input vector gets sorted. */
    double CalculateMedian(std::vector<double>& signals)
    {
      auto size = signals.size();

      if (size == 0) {
        return 0.0;  // Undefined, really.
      } else if (size <= 100) {
        // sort() or partial_sort is in O(NlogN)
        sort(signals.begin(), signals.end());
        if (size % 2 == 0) {
          return (signals[size / 2 - 1] + signals[size / 2]) / 2;
        } else {
          return signals[size / 2];
        }
      } else {
        // nth_element or max_element in O(N) only
        // All elements before the nth are guanranteed smaller
        auto n = size / 2;
        nth_element(signals.begin(), signals.begin() + n, signals.end());
        auto med = signals[n];
        if (!(size & 1)) { // if size is even
          auto max_it = max_element(signals.begin(), signals.begin() + n);
          med = (*max_it + med) / 2.0;
        }
        return med;
      }
    }
    /** Helper function to calculate a median from 1D histogram. */
    double CalculateMedian(TH1* hist)
    {
      double quantiles[1]; // One element just for median
      double probSums[1] = {0.5}; // median definiton
      hist->GetQuantiles(1, quantiles, probSums);
      return quantiles[0];
    }

    /** Helper function to estimate MPV from 1D histogram. */
    double FitLandau(TH1* hist)
    {
      auto size = hist->GetEntries();
      if (size == 0) return 0.0; // Undefined.

      int max = hist->GetBinLowEdge(hist->GetNbinsX() + 1);
      int min = hist->GetBinLowEdge(1);

      // create fit function
      TF1* landau = new TF1("landau", "TMath::Landau(x,[0],[1])*[2]", min, max);
      landau->SetParNames("MPV", "sigma", "scale");
      landau->SetParameters(1., 0.1, 1000);
      landau->SetParLimits(0, 0., 3.);

      Int_t status = hist->Fit("landau", "Lq", "", min, max);
      double MPV = landau->GetParameter("MPV");

      B2INFO("Fit result: " << status << " MPV " <<  MPV << " sigma " << landau->GetParameter("sigma")
             << " scale " << landau->GetParameter("scale") << " chi2 " << landau->GetChisquare());

      // clean up
      delete landau;

      // check fit status
      if (status == 0) return MPV;
      else {
        B2WARNING("Fit failed!. using default value.");
        return 0.0;
      }
    }

    /** Helper function to calculate MPV from a vector. The input vector gets sorted. */
    double FitLandau(std::vector<double>& signals)
    {
      auto size = signals.size();
      if (size == 0) return 0.0; // Undefined, really.

      // get max and min values of signal vector
      double max = *max_element(signals.begin(), signals.end());
      double min = *min_element(signals.begin(), signals.end());

      // create histogram to hold signals and fill it
      TH1D* hist_signals = new TH1D("", "", max - min, min, max);
      for (auto it = signals.begin(); it != signals.end(); ++it) {
        hist_signals->Fill(*it);
      }

      double MPV = FitLandau(hist_signals);
      delete hist_signals;

      return MPV;
    }
  }// namespace PXD
} // namespace Belle2
