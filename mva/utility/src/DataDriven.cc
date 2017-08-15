/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/utility/DataDriven.h>
#include <mva/utility/Utility.h>
#include <mva/methods/PDF.h>
#include <mva/methods/Trivial.h>
#include <mva/methods/Combination.h>

#include <framework/logging/Logger.h>
#include <iostream>

namespace Belle2 {
  namespace MVA {

    ReweightingDataset::ReweightingDataset(const GeneralOptions& general_options, Dataset& dataset,
                                           const std::vector<float>& weights) : Dataset(general_options), m_dataset(dataset), m_weights(weights) { }

    void ReweightingDataset::loadEvent(unsigned int event)
    {
      m_dataset.loadEvent(event);
      m_input = m_dataset.m_input;
      m_spectators = m_dataset.m_spectators;
      m_isSignal = m_dataset.m_isSignal;
      m_target = m_dataset.m_target;
      m_weight = m_weights[event] * m_dataset.m_weight;
    }

    SidebandDataset::SidebandDataset(const GeneralOptions& general_options, Dataset& dataset, Dataset& mc_dataset,
                                     std::string sideband_variable) : Dataset(general_options), m_dataset(dataset)
    {

      m_spectator_index = dataset.getSpectatorIndex(sideband_variable);
      int mc_spectator_index = mc_dataset.getSpectatorIndex(sideband_variable);

      double total_signal_mc = 0.0;
      double total_mc = 0.0;
      double sum_signal_sr = 0.0;
      double sum_sr = 0.0;
      double sum_signal_br = 0.0;
      double sum_br = 0.0;
      double sum_signal_nr = 0.0;
      double sum_nr = 0.0;

      for (unsigned int iEvent = 0; iEvent < mc_dataset.getNumberOfEvents(); ++iEvent) {
        mc_dataset.loadEvent(iEvent);
        if (mc_dataset.m_isSignal)
          total_signal_mc += mc_dataset.m_weight;
        total_mc += mc_dataset.m_weight;
        if (mc_dataset.m_spectators[mc_spectator_index] == 1.0) {
          if (mc_dataset.m_isSignal)
            sum_signal_sr += mc_dataset.m_weight;
          sum_sr += mc_dataset.m_weight;
        } else if (mc_dataset.m_spectators[mc_spectator_index] == 2.0) {
          if (mc_dataset.m_isSignal)
            sum_signal_br += mc_dataset.m_weight;
          sum_br += mc_dataset.m_weight;
        } else if (mc_dataset.m_spectators[mc_spectator_index] == 3.0) {
          if (mc_dataset.m_isSignal)
            sum_signal_nr += mc_dataset.m_weight;
          sum_nr += mc_dataset.m_weight;
        }
      }

      double total_data = 0.0;
      double sum_data_sr = 0.0;
      double sum_data_br = 0.0;
      double sum_data_nr = 0.0;

      for (unsigned int iEvent = 0; iEvent < dataset.getNumberOfEvents(); ++iEvent) {
        dataset.loadEvent(iEvent);
        total_data += dataset.m_weight;
        if (dataset.m_spectators[m_spectator_index] == 1.0) {
          sum_data_sr += dataset.m_weight;
        } else if (dataset.m_spectators[m_spectator_index] == 2.0) {
          sum_data_br += dataset.m_weight;
        } else if (dataset.m_spectators[m_spectator_index] == 3.0) {
          sum_data_nr += dataset.m_weight;
        }
      }

      if (sum_signal_br / sum_br > 0.01) {
        B2WARNING("The background region you defined in the sideband substraction contains more than 1% signal");
      }
      if (sum_signal_nr / sum_nr > 0.01) {
        B2WARNING("The negative signal region you defined in the sideband substraction contains more than 1% signal");
      }

      if (sum_data_sr - sum_signal_sr < 0) {
        B2ERROR("There is less data in the signal region than the expected amount of signal events in the signal region estimated from MC.");
      }

      if (total_data - total_signal_mc < 0) {
        B2ERROR("There is less data than the expected amount of signal events estimated from MC.");
      }

      // We assume the number of signal events is correctly described in mc
      // Everything else (like the background) we take from the data sample

      // So Signal events in the signal region receive weight 1
      m_signal_weight = 1.0;

      // The background is scaled so that it corresponds to the total background in the whole sample
      m_background_weight = (total_data - total_signal_mc) / sum_data_br;

      // The negative signal is scaled so it corresponds to the expected background in the signal region:
      // Background Events in Signal Region in Data = Total Events in Signal Region In Data - Signal Events in Signal Region in MC
      m_negative_signal_weight = - (sum_data_sr - sum_signal_sr) / sum_data_nr;

      B2INFO("Data " << total_data << " " << sum_data_sr << " " << sum_data_br << " " << sum_data_nr);
      B2INFO("MC " << total_mc << " " << sum_sr << " " << sum_br << " " << sum_nr);
      B2INFO("MC (signal)" << total_signal_mc << " " << sum_signal_sr << " " << sum_signal_br << " " << sum_signal_nr);
      B2INFO("Sideband Substraction: Signal Weight " << m_signal_weight << " Background Weight " << m_background_weight <<
             " Negative Signal Weight " << m_negative_signal_weight);

    }

    void SidebandDataset::loadEvent(unsigned int event)
    {
      m_dataset.loadEvent(event);
      m_input = m_dataset.m_input;
      m_spectators = m_dataset.m_spectators;
      m_weight = m_dataset.m_weight;
      if (m_spectators[m_spectator_index] == 1.0) {
        m_isSignal = true;
        m_target = 1.0;
        m_weight *= m_signal_weight;
      } else if (m_spectators[m_spectator_index] == 2.0) {
        m_isSignal = false;
        m_target = 0.0;
        m_weight *= m_background_weight;
      } else if (m_spectators[m_spectator_index] == 3.0) {
        m_isSignal = true;
        m_target = 1.0;
        m_weight *= m_negative_signal_weight;
      } else {
        m_isSignal = false;
        m_target = 0.0;
        m_weight = 0.0;
      }
    }


    SPlotDataset::SPlotDataset(const GeneralOptions& general_options, Dataset& dataset, const std::vector<float>& weights,
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
      // TODO
      // Instead of calculating the covariance matrix directly one could do an actual fit here,
      // this would reduce the reliance on MC even further.
      // Also it should be no problem generalize this code to more than two components.
      // I didn't implement this, yet. Since it is not forseeable if and in which context this will be used,
      // so feel free to implement the missing features here.

      auto discriminants = dataset.getFeature(0);

      // Calculate Covariance Matrix of pdfs by calculating first the inverse-covariance matrix
      // and then inverting the inverse-convariance matrix

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
      double determinante = (inverse_covariance[0] * inverse_covariance[2] - inverse_covariance[1] * inverse_covariance[1]);
      covariance[0] = inverse_covariance[2] / determinante;
      covariance[1] = -inverse_covariance[1] / determinante;
      covariance[2] = inverse_covariance[0] / determinante;

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
