/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/interface/Dataset.h>

#include <analysis/VariableManager/Utility.h>

#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace MVA {

    Dataset::Dataset(const GeneralOptions& general_options) : m_general_options(general_options)
    {
      m_input.resize(m_general_options.m_variables.size(), 0);
      m_target = 0.0;
      m_weight = 1.0;
      m_isSignal = false;
    }

    float Dataset::getSignalFraction()
    {

      double signal_weight_sum = 0;
      double weight_sum = 0;
      for (unsigned int i = 0; i < getNumberOfEvents(); ++i) {
        loadEvent(i);
        weight_sum += m_weight;
        if (m_isSignal)
          signal_weight_sum += m_weight;
      }
      return signal_weight_sum / weight_sum;

    }

    std::vector<float> Dataset::getFeature(unsigned int iFeature)
    {

      std::vector<float> result(getNumberOfEvents());
      for (unsigned int iEvent = 0; iEvent < getNumberOfEvents(); ++iEvent) {
        loadEvent(iEvent);
        result[iEvent] = m_input[iFeature];
      }
      return result;

    }

    std::vector<float> Dataset::getWeights()
    {

      std::vector<float> result(getNumberOfEvents());
      for (unsigned int iEvent = 0; iEvent < getNumberOfEvents(); ++iEvent) {
        loadEvent(iEvent);
        result[iEvent] = m_weight;
      }
      return result;

    }

    std::vector<float> Dataset::getTargets()
    {

      std::vector<float> result(getNumberOfEvents());
      for (unsigned int iEvent = 0; iEvent < getNumberOfEvents(); ++iEvent) {
        loadEvent(iEvent);
        result[iEvent] = m_target;
      }
      return result;

    }

    std::vector<bool> Dataset::getSignals()
    {

      std::vector<bool> result(getNumberOfEvents());
      for (unsigned int iEvent = 0; iEvent < getNumberOfEvents(); ++iEvent) {
        loadEvent(iEvent);
        result[iEvent] = m_isSignal;
      }
      return result;

    }


    SingleDataset::SingleDataset(const GeneralOptions& general_options, const std::vector<float>& input,
                                 float target) : Dataset(general_options)
    {
      m_input = input;
      m_target = target;
      m_weight = 1.0;
      m_isSignal = std::lround(target) == m_general_options.m_signal_class;
    }

    SubDataset::SubDataset(const GeneralOptions& general_options, const std::vector<bool>& events,
                           Dataset& dataset) : Dataset(general_options), m_dataset(dataset)
    {

      for (auto& v : m_general_options.m_variables) {
        auto it = std::find(m_dataset.m_general_options.m_variables.begin(), m_dataset.m_general_options.m_variables.end(), v);
        if (it == m_dataset.m_general_options.m_variables.end()) {
          B2ERROR("Couldn't find variable " << v << " in GeneralOptions");
          throw std::runtime_error("Couldn't find variable " + v + " in GeneralOptions");
        }
        m_feature_indices.push_back(it - m_dataset.m_general_options.m_variables.begin());
      }

      if (events.size() > 0)
        m_use_event_indices = true;

      if (m_use_event_indices) {
        m_event_indices.resize(dataset.getNumberOfEvents());
        unsigned int n_events = 0;
        for (unsigned int iEvent = 0; iEvent < dataset.getNumberOfEvents(); ++iEvent) {
          if (events.size() == 0 or events[iEvent]) {
            m_event_indices[n_events] = iEvent;
            n_events++;
          }
        }
        m_event_indices.resize(n_events);
      }

    }

    void SubDataset::loadEvent(unsigned int iEvent)
    {
      unsigned int index = iEvent;
      if (m_use_event_indices)
        index = m_event_indices[iEvent];
      m_dataset.loadEvent(index);
      m_target = m_dataset.m_target;
      m_weight = m_dataset.m_weight;
      m_isSignal = m_dataset.m_isSignal;

      for (unsigned int iFeature = 0; iFeature < m_input.size(); ++iFeature) {
        m_input[iFeature] = m_dataset.m_input[m_feature_indices[iFeature]];
      }

    }

    std::vector<float> SubDataset::getFeature(unsigned int iFeature)
    {

      auto v = m_dataset.getFeature(m_feature_indices[iFeature]);
      if (not m_use_event_indices)
        return v;
      std::vector<float> result(m_event_indices.size());
      for (unsigned int iEvent = 0; iEvent < getNumberOfEvents(); ++iEvent) {
        result[iEvent] = v[m_event_indices[iEvent]];
      }
      return result;

    }

    ROOTDataset::ROOTDataset(const GeneralOptions& general_options) : Dataset(general_options)
    {
      m_file = TFile::Open(m_general_options.m_datafile.c_str(), "UPDATE");
      if (not m_file or not m_file->IsOpen()) {
        B2ERROR("Error during open of ROOT file named " << m_general_options.m_datafile);
        throw std::runtime_error("Error during open of ROOT file named " + m_general_options.m_datafile);
      }
      m_file->cd();
      m_file->GetObject(m_general_options.m_treename.c_str(), m_tree);
      setBranchAddresses();
    }

    void ROOTDataset::loadEvent(unsigned int event)
    {
      m_tree->GetEvent(event, 1);
      m_isSignal = std::lround(m_target) == m_general_options.m_signal_class;
    }

    std::vector<float> ROOTDataset::getFeature(unsigned int iFeature)
    {
      std::string branchName = Belle2::Variable::makeROOTCompatible(m_general_options.m_variables[iFeature]);
      int nentries = m_tree->GetEntries();
      std::vector<float> values(nentries);

      float object;
      m_tree->SetBranchStatus("*", 0);
      m_tree->SetBranchStatus(branchName.c_str(), 1);
      m_tree->SetBranchAddress(branchName.c_str(), &object);
      for (int i = 0; i < nentries; ++i) {
        m_tree->GetEvent(i);
        values[i] = object;
      }
      m_tree->SetBranchStatus("*", 1);
      setBranchAddresses();
      return values;
    }

    ROOTDataset::~ROOTDataset()
    {
      /* This leads to a segmentation fault,
       * it is not clear why.
       * if(m_tree) {
        m_tree->SetDirectory(nullptr);
      }*/
      if (m_file) {
        m_file->Close();
      }
      m_tree = nullptr;
      m_file = nullptr;
    }

    void ROOTDataset::setBranchAddresses()
    {
      if (m_general_options.m_weight_variable == "__weight__") {
        m_tree->SetBranchAddress("__weight__", &m_weight);
      } else if (not m_general_options.m_weight_variable.empty()) {
        m_tree->SetBranchAddress(Belle2::Variable::makeROOTCompatible(m_general_options.m_weight_variable).c_str(), &m_weight);
      }
      if (not m_general_options.m_target_variable.empty()) {
        m_tree->SetBranchAddress(Belle2::Variable::makeROOTCompatible(m_general_options.m_target_variable).c_str(), &m_target);
      }
      for (unsigned int i = 0; i < m_general_options.m_variables.size(); ++i)
        m_tree->SetBranchAddress(Belle2::Variable::makeROOTCompatible(m_general_options.m_variables[i]).c_str(), &m_input[i]);
    }

  }
}
