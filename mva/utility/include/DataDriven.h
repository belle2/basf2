/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef INCLUDE_GUARD_BELLE2_MVA_SPLOT_HEADER
#define INCLUDE_GUARD_BELLE2_MVA_SPLOT_HEADER

#include <mva/interface/Dataset.h>
#include <mva/utility/Binning.h>
#include <mva/interface/Expert.h>


namespace Belle2 {
  namespace MVA {

    /**
     * Dataset for Reweighting
     * Wraps a dataset and provides each data-point with a new weight
     */
    class ReweightingDataset : public Dataset {

    public:
      /**
       * Constructs a new ReweightingDataset
       * @param general_options shared options defining the dataset (variables, ...)
       * @param dataset containing the data-points
       * @param weights reweighting weights
       */
      ReweightingDataset(const GeneralOptions& general_options, Dataset& dataset, const std::vector<float>& weights);

      /**
       * Returns the number of features in this dataset
       */
      virtual unsigned int getNumberOfFeatures() const override { return m_dataset.getNumberOfFeatures(); }

      /**
       * Returns the number of features in this dataset
       */
      virtual unsigned int getNumberOfSpectators() const override { return m_dataset.getNumberOfSpectators(); }

      /**
       * Returns the number of events in this dataset
       */
      virtual unsigned int getNumberOfEvents() const override { return m_dataset.getNumberOfEvents(); };

      /**
       * Returns all values of one feature in a std::vector<float>
       * @param iFeature the position of the feature to return
       */
      virtual std::vector<float> getFeature(unsigned int iFeature) override { return m_dataset.getFeature(iFeature); }

      /**
       * Returns all values of one spectator in a std::vector<float>
       * @param iSpectator the position of the feature to return
       */
      virtual std::vector<float> getSpectator(unsigned int iSpectator) override { return m_dataset.getSpectator(iSpectator); }

      /**
       * Returns all weights
       */
      virtual std::vector<float> getWeights() override
      {
        auto weights = m_dataset.getWeights();
        for (unsigned int iEvent = 0; iEvent < weights.size(); ++iEvent) {
          weights[iEvent] *= m_weights[iEvent];
        }
        return weights;
      }

      /**
       * Returns all targets
       */
      virtual std::vector<float> getTargets() override { return m_dataset.getTargets(); }

      /**
       * Returns all is Signals
       */
      virtual std::vector<bool> getSignals() override { return m_dataset.getSignals(); }

      /**
       * Load the event number iEvent
       * @param iEvent event number to load
       */
      virtual void loadEvent(unsigned int event) override;

    private:
      Dataset& m_dataset; /**< Wrapped dataset */
      std::vector<float> m_weights; /**< sPlot weights */
    };

    /**
     * Dataset for Sideband Substraction
     * Wraps a dataset and provides each data-point with a new weight
     */
    class SidebandDataset : public Dataset {

    public:
      /**
       * Constructs a new SidebandDataset
       * @param general_options shared options defining the dataset (variables, ...)
       * @param dataset containing the data-points
       * @param mc_dataset containing mc
       * @param sideband_variable defining the sideband regions
       */
      SidebandDataset(const GeneralOptions& general_options, Dataset& dataset, Dataset& mc_dataset, std::string sideband_variable);

      /**
       * Returns the number of features in this dataset
       */
      virtual unsigned int getNumberOfFeatures() const override { return m_dataset.getNumberOfFeatures(); }

      /**
       * Returns the number of features in this dataset
       */
      virtual unsigned int getNumberOfSpectators() const override { return m_dataset.getNumberOfSpectators(); }

      /**
       * Returns the number of events in this dataset
       */
      virtual unsigned int getNumberOfEvents() const override { return m_dataset.getNumberOfEvents(); };

      /**
       * Returns all values of one feature in a std::vector<float>
       * @param iFeature the position of the feature to return
       */
      virtual std::vector<float> getFeature(unsigned int iFeature) override { return m_dataset.getFeature(iFeature); }

      /**
       * Returns all values of one spectator in a std::vector<float>
       * @param iSpectator the position of the feature to return
       */
      virtual std::vector<float> getSpectator(unsigned int iSpectator) override { return m_dataset.getSpectator(iSpectator); }

      /**
       * Load the event number iEvent
       * @param iEvent event number to load
       */
      virtual void loadEvent(unsigned int event) override;

    private:
      Dataset& m_dataset; /**< Wrapped dataset */
      int m_spectator_index; /**< spectator containing the sideband variable */
      double m_signal_weight; /**< the weight for signal events */
      double m_background_weight; /**< the weight for background events */
      double m_negative_signal_weight; /**< the weight for negative signal events */
    };

    /**
     * Dataset for sPlot
     * Wraps a dataset and provides each data-point twice, once as signal and once as background
     */
    class SPlotDataset : public Dataset {

    public:
      /**
       * Constructs a new SPlotDataset
       * @param general_options shared options defining the dataset (variables, ...)
       * @param dataset containing the data-points
       * @param weights sPlot weights
       * @param signalFraction original signal fraction
       */
      SPlotDataset(const GeneralOptions& general_options, Dataset& dataset, const std::vector<float>& weights, float signalFraction);

      /**
       * Returns the number of features in this dataset
       */
      virtual unsigned int getNumberOfFeatures() const override { return m_dataset.getNumberOfFeatures(); }

      /**
       * Returns the number of features in this dataset
       */
      virtual unsigned int getNumberOfSpectators() const override { return m_dataset.getNumberOfSpectators(); }

      /**
       * Returns the number of events in this dataset
       */
      virtual unsigned int getNumberOfEvents() const override { return 2 * m_dataset.getNumberOfEvents(); };

      /**
       * Load the event number iEvent
       * @param iEvent event number to load
       */
      virtual void loadEvent(unsigned int event) override;

      /**
       * Returns the signal fraction of the whole sample
       */
      virtual float getSignalFraction() override;

    private:
      Dataset& m_dataset; /**< Wrapped dataset */
      std::vector<float> m_weights; /**< sPlot weights */
      float m_signalFraction; /**< Signal fraction */
    };

    /**
     * Returns sPlot weights
     * @param dataset to calculate the weights for
     * @param binning used to retrieve the required pdf and cdf values
     */
    std::vector<float> getSPlotWeights(Dataset& dataset, Binning& binning);

    /**
     * Returns Boost weights
     * @param dataset to calculate the weights for
     * @param binning used to retrieve the required pdf and cdf values
     */
    std::vector<float> getBoostWeights(Dataset& dataset, Binning& binning);

    /**
     * Returns advanced sPlot weights
     * @param dataset to calculate the weights for
     * @param binning used to retrieve the required pdf and cdf values
     * @param boost_prediction prediction of the expert for the dataset
     */
    std::vector<float> getAPlotWeights(Dataset& dataset, Binning& binning, const std::vector<float>& boost_prediction);

  }
}
#endif
