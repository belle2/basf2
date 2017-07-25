/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/utility/Binning.h>

#include <algorithm>
#include <numeric>

namespace Belle2 {
  namespace MVA {

    Binning::Binning(unsigned int nBins)
    {

      m_signal_pdf.resize(nBins, 0.0);
      m_signal_cdf.resize(nBins, 0.0);
      m_bckgrd_pdf.resize(nBins, 0.0);
      m_bckgrd_cdf.resize(nBins, 0.0);
      m_boundaries.resize(nBins + 1, 0.0);

      m_signal_yield = 0;
      m_bckgrd_yield = 0;
    }

    unsigned int Binning::getBin(float datapoint) const
    {

      auto it = std::upper_bound(m_boundaries.begin(), m_boundaries.end(), datapoint);
      unsigned int bin = std::distance(m_boundaries.begin(), it);
      if (bin == 0)
        bin = 1;
      if (bin == m_boundaries.size())
        bin = m_boundaries.size() - 1;
      return bin - 1;

    }

    void Binning::normalizePDFs()
    {

      unsigned int nBins = m_signal_pdf.size();

      m_signal_yield = 0;
      m_bckgrd_yield = 0;

      // Total number of events
      for (unsigned int iBin = 0; iBin < nBins; ++iBin) {
        m_signal_yield += m_signal_pdf[iBin];
        m_bckgrd_yield += m_bckgrd_pdf[iBin];
      }

      // Each bin is normed to its width
      double last_valid_bound = m_boundaries[0];
      for (unsigned int iBin = 0; iBin < nBins; ++iBin) {
        m_signal_pdf[iBin] /= m_signal_yield * (m_boundaries[iBin + 1] - last_valid_bound) / (m_boundaries[nBins] - m_boundaries[0]);
        m_bckgrd_pdf[iBin] /= m_bckgrd_yield * (m_boundaries[iBin + 1] - last_valid_bound) / (m_boundaries[nBins] - m_boundaries[0]);
        if (iBin + 1 < nBins and m_boundaries[iBin + 2] > m_boundaries[iBin + 1]) {
          last_valid_bound = m_boundaries[iBin + 1];
        }
      }

    }

    void Binning::calculateCDFsFromPDFs()
    {

      unsigned int nBins = m_signal_pdf.size();

      m_signal_cdf = m_signal_pdf;
      m_bckgrd_cdf = m_bckgrd_pdf;

      for (unsigned int iBin = 0; iBin < nBins; ++iBin) {
        m_signal_cdf[iBin] *= (m_boundaries[iBin + 1] - m_boundaries[iBin]) / (m_boundaries[nBins] - m_boundaries[0]);
        m_bckgrd_cdf[iBin] *= (m_boundaries[iBin + 1] - m_boundaries[iBin]) / (m_boundaries[nBins] - m_boundaries[0]);
      }

      for (unsigned int iBin = 1; iBin < nBins; ++iBin) {
        m_signal_cdf[iBin] += m_signal_cdf[iBin - 1];
        m_bckgrd_cdf[iBin] += m_bckgrd_cdf[iBin - 1];
      }

    }

    Binning Binning::CreateEqualFrequency(const std::vector<float>& data, const std::vector<float>& weights,
                                          const std::vector<bool>& isSignal, unsigned int nBins)
    {

      Binning binning(nBins);

      unsigned int nEvents = data.size();

      std::vector<unsigned int> indices(nEvents);
      std::iota(indices.begin(), indices.end(), 0);
      std::sort(indices.begin(), indices.end(), [&](unsigned int i, unsigned int j) {return data[i] < data[j]; });

      double sum_weights = 0;
      for (auto& w : weights)
        sum_weights += w;
      double weight_per_bin = sum_weights / nBins;

      unsigned int bin = 1;
      double current_weight = 0;
      binning.m_boundaries[0] = data[indices[0]];
      binning.m_boundaries[nBins] = data[indices[nEvents - 1]];

      for (unsigned int iEvent = 0; iEvent < nEvents; ++iEvent) {
        unsigned int index = indices[iEvent];
        current_weight += weights[index];
        if (current_weight >= weight_per_bin and bin < nBins and binning.m_boundaries[bin - 1] < data[index]) {
          unsigned int number_of_bins = static_cast<unsigned int>(current_weight / weight_per_bin);
          current_weight -= weight_per_bin * number_of_bins;
          for (unsigned int i = 0; i < number_of_bins; ++i) {
            binning.m_boundaries[bin] = data[index];
            bin++;
          }
        }
        if (isSignal[index]) {
          binning.m_signal_pdf[bin - 1] += weights[index];
        } else {
          binning.m_bckgrd_pdf[bin - 1] += weights[index];
        }
      }

      binning.normalizePDFs();
      binning.calculateCDFsFromPDFs();

      return binning;
    }

    Binning Binning::CreateEquidistant(const std::vector<float>& data, const std::vector<float>& weights,
                                       const std::vector<bool>& isSignal, unsigned int nBins)
    {

      Binning binning(nBins);

      auto minmax = std::minmax_element(data.begin(), data.end());
      float min = *(minmax.first);
      float max = *(minmax.second);
      float step = (max - min) / nBins;

      for (unsigned int iBin = 0; iBin <= nBins; ++iBin) {
        binning.m_boundaries[iBin] = min + step * iBin;
      }

      for (unsigned int iEvent = 0; iEvent < data.size(); ++iEvent) {
        unsigned int bin = binning.getBin(data[iEvent]);

        if (isSignal[iEvent])
          binning.m_signal_pdf[bin] += weights[iEvent];
        else
          binning.m_bckgrd_pdf[bin] += weights[iEvent];

      }

      binning.normalizePDFs();
      binning.calculateCDFsFromPDFs();

      return binning;

    }

  }
}

