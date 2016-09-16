/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/utility/SPlot.h>
#include <mva/utility/Utility.h>
#include <mva/methods/PDF.h>
#include <mva/methods/Trivial.h>
#include <mva/methods/Combination.h>

#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace MVA {

    SPlotDataset::SPlotDataset(GeneralOptions& general_options, Dataset& dataset, const std::vector<float>& weights,
                               float signalFraction) : Dataset(general_options), m_dataset(dataset), m_weights(weights), m_signalFraction(signalFraction) { }


    float SPlotDataset::getSignalFraction()
    {
      return m_signalFraction;
    }

    void SPlotDataset::loadEvent(unsigned int event)
    {
      m_dataset.loadEvent(event / 2);
      for (unsigned int iFeature = 0; iFeature < getNumberOfFeatures(); ++iFeature) {
        m_input[iFeature] = m_dataset.m_input[iFeature];
      }

      if (event % 2 == 1) {
        m_isSignal = false;
        m_target = 0.0;
        m_weight = m_weights[event] * m_dataset.m_weight;
      } else {
        m_isSignal = true;
        m_target = 1.0;
        m_weight = m_weights[event] * m_dataset.m_weight;
      }
    }


    std::vector<float> getSPlotWeights(Dataset& dataset, Binning& binning)
    {

      auto discriminants = dataset.getFeature(0);

      // Calculate Covariance Matrix of pdfs by calculating first the inverse-covariance matrix
      // and then inverting the inverse-convariance matrix
      //
      double inverse_covariance[3] = {0};
      for (auto& v : discriminants) {
        const unsigned int iBin = binning.getBin(v);
        double norm = (binning.m_signal_yield * binning.m_signal_pdf[iBin] + binning.m_bckgrd_yield * binning.m_bckgrd_pdf[iBin]);
        norm = norm * norm;
        inverse_covariance[0] += binning.m_signal_pdf[iBin] * binning.m_signal_pdf[iBin] / norm;
        inverse_covariance[1] += binning.m_signal_pdf[iBin] * binning.m_bckgrd_pdf[iBin] / norm;
        inverse_covariance[2] += binning.m_bckgrd_pdf[iBin] * binning.m_bckgrd_pdf[iBin] / norm;
      }
      double covariance[3] = {0};
      double norm = (inverse_covariance[0] * inverse_covariance[2] - inverse_covariance[1] * inverse_covariance[1]);
      covariance[0] = inverse_covariance[2] / norm;
      covariance[1] = -inverse_covariance[1] / norm;
      covariance[2] = inverse_covariance[0] / norm;

      // Finally calculate sPlot weights
      std::vector<float> splot_weights;
      splot_weights.reserve(2 * discriminants.size());
      for (auto& v : discriminants) {
        const unsigned int iBin = binning.getBin(v);
        double norm = (binning.m_signal_yield * binning.m_signal_pdf[iBin] + binning.m_bckgrd_yield * binning.m_bckgrd_pdf[iBin]);
        splot_weights.push_back((covariance[0] * binning.m_signal_pdf[iBin] + covariance[1] * binning.m_bckgrd_pdf[iBin]) / norm);
        splot_weights.push_back((covariance[1] * binning.m_signal_pdf[iBin] + covariance[2] * binning.m_bckgrd_pdf[iBin]) / norm);
      }

      B2INFO("Covariance Matrix of SPlot");
      B2INFO(covariance[0] << " " << covariance[1] << " " << covariance[2]);

      return splot_weights;

    }

    std::vector<float> getBoostWeights(Dataset& dataset, Binning& binning)
    {

      std::vector<float> boost_weights;
      boost_weights.reserve(2 * dataset.getNumberOfEvents());
      for (unsigned int iEvent = 0; iEvent < dataset.getNumberOfEvents(); ++iEvent) {
        dataset.loadEvent(iEvent);
        const unsigned int bin = binning.getBin(dataset.m_input[0]);
        boost_weights.push_back(binning.m_signal_cdf[bin] / binning.m_bckgrd_pdf[bin]);
        boost_weights.push_back((1 - binning.m_signal_cdf[bin]) / binning.m_bckgrd_pdf[bin]);
      }
      return boost_weights;

    }

    std::vector<float> getAPlotWeights(Dataset& dataset, Binning& binning, const std::vector<float>& boost_predictions)
    {

      std::vector<float> splot_weights = getSPlotWeights(dataset, binning);
      std::vector<float> aplot_weights;
      aplot_weights.reserve(2 * dataset.getNumberOfEvents());

      for (unsigned int iEvent = 0; iEvent < dataset.getNumberOfEvents(); ++iEvent) {
        dataset.loadEvent(iEvent);
        const unsigned int iBin = binning.getBin(dataset.m_input[0]);

        double reg_boost_prediction = boost_predictions[iEvent];

        if (reg_boost_prediction > 0.995)
          reg_boost_prediction = 0.995;

        if (reg_boost_prediction < 0.005)
          reg_boost_prediction = 0.005;

        float aplot_weight = (binning.m_signal_cdf[iBin] / reg_boost_prediction + (1 - binning.m_signal_cdf[iBin]) /
                              (1 - reg_boost_prediction)) / 2.0;

        aplot_weights.push_back(splot_weights[2 * iEvent] * aplot_weight);
        aplot_weights.push_back(splot_weights[2 * iEvent + 1] * aplot_weight);

      }

      return aplot_weights;
    }


  }
}
