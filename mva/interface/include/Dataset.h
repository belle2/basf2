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
#ifndef INCLUDE_GUARD_BELLE2_MVA_DATASET_HEADER
#define INCLUDE_GUARD_BELLE2_MVA_DATASET_HEADER

#include <mva/interface/Options.h>

#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TLeaf.h>
#include <TROOT.h>

#include <string>

namespace Belle2 {
  namespace MVA {

    /**
     * Abstract base class of all Datasets given to the MVA interface
     * The current event can always be accessed via the public members of this class
     */
    class Dataset {

    public:
      /**
       * Constructs a new dataset given the general options
       * @param general_options which defines e.g. number of variables
       */
      explicit Dataset(const GeneralOptions& general_options);

      /**
       * Virtual default destructor
       */
      virtual ~Dataset() = default;

      /**
       * Returns the number of features in this dataset
       */
      virtual unsigned int getNumberOfFeatures() const = 0;

      /**
       * Returns the number of spectators in this dataset
       */
      virtual unsigned int getNumberOfSpectators() const = 0;

      /**
       * Returns the number of events in this dataset
       */
      virtual unsigned int getNumberOfEvents() const = 0;

      /**
       * Load the event number iEvent
       * @param iEvent event number to load
       */
      virtual void loadEvent(unsigned int iEvent) = 0;

      /**
       * Returns the signal fraction of the whole sample
       */
      virtual float getSignalFraction();

      /**
       * Return index of feature with the given name
       * @param feature name of the feature
       */
      virtual unsigned int getFeatureIndex(std::string feature);

      /**
       * Return index of spectator with the given name
       * @param spectator name of the spectator
       */
      virtual unsigned int getSpectatorIndex(std::string spectator);

      /**
       * Returns all values of one feature in a std::vector<float>
       * @param iFeature the position of the feature to return
       */
      virtual std::vector<float> getFeature(unsigned int iFeature);

      /**
       * Returns all values of one spectator in a std::vector<float>
       * @param iSpectator the position of the feature to return
       */
      virtual std::vector<float> getSpectator(unsigned int iSpectator);

      /**
       * Returns all weights
       */
      virtual std::vector<float> getWeights();

      /**
       * Returns all targets
       */
      virtual std::vector<float> getTargets();

      /**
       * Returns all is Signals
       */
      virtual std::vector<bool> getSignals();

      GeneralOptions m_general_options; /**< GeneralOptions passed to this dataset */
      std::vector<float> m_input; /**< Contains all feature values of the currently loaded event */
      std::vector<float> m_spectators; /**< Contains all spectators values of the currently loaded event */
      float m_weight; /**< Contains the weight of the currently loaded event */
      float m_target; /**< Contains the target value of the currently loaded event */
      bool m_isSignal; /**< Defines if the currently loaded event is signal or background */
    };


    /**
     * Wraps the data of a single event into a Dataset.
     * Mostly useful if one wants to apply an Expert to a single feature vector
     */
    class SingleDataset : public Dataset {

    public:
      /**
       * Constructs a new SingleDataset
       * @param general_options which defines e.g. number of variables
       * @param input feature values of the single event
       * @param spectators spectator values of the single event
       * @param target target value of the single event (defaults to 1, because often this is not known if one wants to apply an expert)
       */
      SingleDataset(const GeneralOptions& general_options, const std::vector<float>& input, float target = 1.0,
                    const std::vector<float>& spectators = std::vector<float>());

      /**
       * Returns the number of features in this dataset
       */
      virtual unsigned int getNumberOfFeatures() const override { return m_input.size(); }

      /**
       * Returns the number of features in this dataset
       */
      virtual unsigned int getNumberOfSpectators() const override { return m_spectators.size(); }

      /**
       * Returns the number of events in this dataset which is always one
       */
      virtual unsigned int getNumberOfEvents() const override { return 1; }

      /**
       * Does nothing in the case of a single dataset, because the only event is already loaded
       */
      virtual void loadEvent(unsigned int) override { };

      /**
       * Returns all values (in this case only one) of one feature in a std::vector<float>
       * @param iFeature the position of the feature to return
       */
      virtual std::vector<float> getFeature(unsigned int iFeature) override { return std::vector<float> {m_input[iFeature]}; }

      /**
       * Returns all values (in this case only one) of one spectator in a std::vector<float>
       * @param iSpectator the position of the spectator to return
       */
      virtual std::vector<float> getSpectator(unsigned int iSpectator) override { return std::vector<float> {m_spectators[iSpectator]}; }

    };

    /**
     * Wraps the data of a multiple event into a Dataset.
     * Mostly useful if one wants to apply an Expert to a feature matrix
     */
    class MultiDataset : public Dataset {

    public:
      /**
       * Constructs a new MultiDataset
       * @param general_options which defines e.g. number of variables
       * @param input feature values of the single event
       * @param spectators spectator values of the single event
       * @param target target value of the single event (defaults to 1, because often this is not known if one wants to apply an expert)
       */
      MultiDataset(const GeneralOptions& general_options, const std::vector<std::vector<float>>& input,
                   const std::vector<std::vector<float>>& spectators,
                   const std::vector<float>& targets = {}, const std::vector<float>& weights = {});

      /**
       * Returns the number of features in this dataset
       */
      virtual unsigned int getNumberOfFeatures() const override { return m_input.size(); }

      /**
       * Returns the number of spectators in this dataset
       */
      virtual unsigned int getNumberOfSpectators() const override { return m_spectators.size(); }

      /**
       * Returns the number of events in this dataset
       */
      virtual unsigned int getNumberOfEvents() const override { return m_matrix.size(); }

      /**
       * Does nothing in the case of a single dataset, because the only event is already loaded
       */
      virtual void loadEvent(unsigned int iEvent) override;


    private:
      std::vector<std::vector<float>> m_matrix; /**< Feature matrix */
      std::vector<std::vector<float>> m_spectator_matrix; /**< Spectator matrix */
      std::vector<float> m_targets; /**< target vector */
      std::vector<float> m_weights; /**< weight vector */

    };

    /**
     * Wraps another Dataset and provides a view to a subset of its features and events.
     * Used by the Combination method which can combine multiple methods with possibly different variables
     */
    class SubDataset : public Dataset {

    public:
      /**
       * Constructs a new SubDataset holding a reference to the wrapped Dataset
       * @param general_options which defines e.g. a subset of variables of the original dataset
       * @param events subset of events which are provided by this Dataset
       * @param dataset reference to the wrapped Dataset
       */
      SubDataset(const GeneralOptions& general_options, const std::vector<bool>& events, Dataset& dataset);

      /**
       * Returns the number of features in this dataset, so the size of the given subset of the variables
       */
      virtual unsigned int getNumberOfFeatures() const override { return m_feature_indices.size(); }

      /**
       * Returns the number of spectators in this dataset, so the size of the given subset of the spectators
       */
      virtual unsigned int getNumberOfSpectators() const override { return m_spectator_indices.size(); }

      /**
       * Returns the number of events in the wrapped dataset
       */
      virtual unsigned int getNumberOfEvents() const override { return m_use_event_indices ? m_event_indices.size() : m_dataset.getNumberOfEvents(); }

      /**
       * Load the event number iEvent from the wrapped dataset
       * @param iEvent event number to load
       */
      virtual void loadEvent(unsigned int iEvent) override;

      /**
       * Returns all values of one feature in a std::vector<float> of the wrapped dataset
       * @param iFeature the position of the feature to return in the given subset
       */
      virtual std::vector<float> getFeature(unsigned int iFeature) override;

      /**
       * Returns all values of one spectator in a std::vector<float> of the wrapped dataset
       * @param iSpectator the position of the spectator to return in the given subset
       */
      virtual std::vector<float> getSpectator(unsigned int iSpectator) override;

    private:
      bool m_use_event_indices = false; /**< Use only a subset of the wrapped dataset events */
      std::vector<unsigned int>
      m_feature_indices; /**< Mapping from the position of a feature in the given subset to its position in the wrapped dataset */
      std::vector<unsigned int>
      m_spectator_indices; /**< Mapping from the position of a spectator in the given subset to its position in the wrapped dataset */
      std::vector<unsigned int>
      m_event_indices; /**< Mapping from the position of a event in the given subset to its position in the wrapped dataset */
      Dataset& m_dataset; /**< Reference to the wrapped dataset */

    };

    /**
     * Wraps two other Datasets, one containing signal, the other background events
     * Used by the reweighting method to train mc files against data files
     */
    class CombinedDataset : public Dataset {

    public:
      /**
       * Constructs a new CombinedDataset holding a reference to the wrapped Datasets
       * @param general_options
       * @param signal_dataset reference to the wrapped Dataset containing signal events
       * @param background_dataset reference to the wrapped Dataset containing background events
       */
      CombinedDataset(const GeneralOptions& general_options, Dataset& signal_dataset, Dataset& background_dataset);

      /**
       * Returns the number of features in this dataset, so the size of the given subset of the variables
       */
      virtual unsigned int getNumberOfFeatures() const override { return m_signal_dataset.getNumberOfFeatures(); }

      /**
       * Returns the number of spectators in this dataset, so the size of the given subset of the spectators
       */
      virtual unsigned int getNumberOfSpectators() const override { return m_signal_dataset.getNumberOfSpectators(); }

      /**
       * Returns the number of events in the wrapped dataset
       */
      virtual unsigned int getNumberOfEvents() const override { return m_signal_dataset.getNumberOfEvents() + m_background_dataset.getNumberOfEvents(); }

      /**
       * Load the event number iEvent from the wrapped dataset
       * @param iEvent event number to load
       */
      virtual void loadEvent(unsigned int iEvent) override;

      /**
       * Returns all values of one feature in a std::vector<float> of the wrapped dataset
       * @param iFeature the position of the feature to return in the given subset
       */
      virtual std::vector<float> getFeature(unsigned int iFeature) override;

      /**
       * Returns all values of one spectator in a std::vector<float> of the wrapped dataset
       * @param iSpectator the position of the spectator to return in the given subset
       */
      virtual std::vector<float> getSpectator(unsigned int iSpectator) override;

    private:
      Dataset& m_signal_dataset; /**< Reference to the wrapped dataset containing signal events */
      Dataset& m_background_dataset; /**< Reference to the wrapped dataset containing background events */

    };


    /**
     * Proivdes a dataset from a ROOT file
     * This is the usually used dataset providing training data to the mva methods
     */
    class ROOTDataset : public Dataset {

    public:
      /**
       * Creates a new ROOTDataset
       * @param _general_options defines the rootfile, treename, branches, ...
       */
      explicit ROOTDataset(const GeneralOptions& _general_options);

      /**
       * Returns the number of features in this dataset
       */
      virtual unsigned int getNumberOfFeatures() const override { return m_input.size(); }

      /**
       * Returns the number of features in this dataset
       */
      virtual unsigned int getNumberOfSpectators() const override { return m_spectators.size(); }

      /**
       * Returns the number of events in this dataset
       */
      virtual unsigned int getNumberOfEvents() const override
      {
        return (m_general_options.m_max_events == 0) ? m_tree->GetEntries() : m_general_options.m_max_events;
      }

      /**
       * Load the event number iEvent from the TTree
       * @param iEvent event number to load
       */
      virtual void loadEvent(unsigned int event) override;

      /**
       * Returns all values of one feature in a std::vector<float>
       * @param iFeature the position of the feature to return
       */
      virtual std::vector<float> getFeature(unsigned int iFeature) override;

      /**
       * Returns all values of of the weights in a std::vector<float>
       */
      virtual std::vector<float> getWeights() override;

      /**
       * Returns all values of one spectator in a std::vector<float>
       * @param iSpectator the position of the spectator to return
       */
      virtual std::vector<float> getSpectator(unsigned int iSpectator) override;

      /**
       * Returns all values for a specified variableType and branchName. The values are read from a root file.
       * The type is inferred from the given memberVariableTarget name.
       * @tparam T type memberVariable of this class which has to be updated (float, double)
       * @param variableType defines {feature, weights, spectator, target}
       * @param branchName name of the branch to read
       * @param memberVariableTarget variable the branch address from the root file is set to
       * @return filled vector from a branch, converted to float
       */
      template<class T>
      std::vector<float> getVectorFromTTree(std::string& variableType, std::string& branchName, T& memberVariableTarget);

      /**
       * Tries to infer the data-type of a root file and sets m_isDoubleInputType
       */
      void setRootInputType();

      /**
       * sets the branch address for a scalar variable to a given target
       * @tparam T target type (float, double)
       * @param variableType defines {feature, weights, spectator, target}
       * @param variableName name of the variable, usually defined in general_options
       * @param variableTarget variable, the address is set to
       */
      template<class T>
      void setScalarVariableAddress(std::string& variableType, std::string& variableName, T& variableTarget);

      /**
       * sets the branch address for a vector variable to a given target
       * @tparam T target type (std::vector<float>, std::vector<double>)
       * @param variableType defines {feature, weights, spectator, target}
       * @param variableName names of the variable, usually defined in general_options
       * @param variableTargets variables, the address is set to
       */
      template<class T>
      void setVectorVariableAddress(std::string& variableType, std::vector<std::string>& variableName,
                                    T& variableTargets);

      /**
       * Virtual destructor
       */
      virtual ~ROOTDataset();


    private:
      /**
       * Sets the branch addresses of all features, weight and target again
       */
      void setBranchAddresses();

      /**
       * Checks if the given branchname exists in the TTree
       * @param tree
       * @param branchname
       */
      bool checkForBranch(TTree*, const std::string&) const;

    protected:
      TChain* m_tree = nullptr; /**< Pointer to the TChain containing the data */
      bool m_isDoubleInputType = true; /**< Defines the expected datatype in the ROOT file */
      std::vector<double> m_input_double; /**< Contains all feature values of the currently loaded event */
      std::vector<double> m_spectators_double; /**< Contains all spectators values of the currently loaded event */
      double m_weight_double; /**< Contains the weight of the currently loaded event */
      double m_target_double; /**< Contains the target value of the currently loaded event */
    };

  }
}
#endif
