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

#include <framework/utilities/MakeROOTCompatible.h>
#include <framework/logging/Logger.h>
#include <framework/io/RootIOUtilities.h>

#include <TDirectory.h>

#include <boost/filesystem/operations.hpp>

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

    MultiDataset::MultiDataset(const GeneralOptions& general_options, const std::vector<std::vector<float>>& matrix,
                               const std::vector<float>& targets, const std::vector<float>& weights) : Dataset(general_options),  m_matrix(matrix),
      m_targets(targets), m_weights(weights)
    {

      if (m_targets.size() > 0 and m_matrix.size() != m_targets.size()) {
        B2ERROR("Feature matrix and target vector need same number of elements in MultiDataset, got " << m_targets.size() << " and " <<
                m_matrix.size());
      }
      if (m_weights.size() > 0 and m_matrix.size() != m_weights.size()) {
        B2ERROR("Feature matrix and weight vector need same number of elements in MultiDataset, got " << m_weights.size() << " and " <<
                m_matrix.size());
      }
    }


    void MultiDataset::loadEvent(unsigned int iEvent)
    {
      m_input = m_matrix[iEvent];

      if (m_targets.size() > 0) {
        m_target = m_targets[iEvent];
        m_isSignal = std::lround(m_target) == m_general_options.m_signal_class;
      }

      if (m_weights.size() > 0)
        m_weight = m_weights[iEvent];

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
      auto filenames = RootIOUtilities::expandWordExpansions(m_general_options.m_datafiles);
      if (filenames.empty()) {
        B2ERROR("Found no valid filenames in GeneralOptions");
        throw std::runtime_error("Found no valid filenames in GeneralOptions");
      }

      //Open TFile
      TDirectory* dir = gDirectory;
      for (const auto& filename : filenames) {
        if (not boost::filesystem::exists(filename)) {
          B2ERROR("Error given ROOT file dies not exists " << filename);
          throw std::runtime_error("Error during open of ROOT file named " + filename);
        }

        TFile* f = TFile::Open(filename.c_str(), "READ");
        if (!f or f->IsZombie() or not f->IsOpen()) {
          B2ERROR("Error during open of ROOT file named " << filename);
          throw std::runtime_error("Error during open of ROOT file named " + filename);
        }
        delete f;
      }
      dir->cd();

      m_tree = new TChain(m_general_options.m_treename.c_str());
      for (const auto& filename : filenames) {
        //nentries = -1 forces AddFile() to read headers
        if (!m_tree->AddFile(filename.c_str(), -1)) {
          B2ERROR("Error during open of ROOT file named " << filename << " cannot retreive tree named " <<
                  m_general_options.m_treename);
          throw std::runtime_error("Error during open of ROOT file named " + filename + " cannot retreive tree named " +
                                   m_general_options.m_treename);
        }
      }

      setBranchAddresses();
    }

    void ROOTDataset::loadEvent(unsigned int event)
    {
      m_tree->GetEvent(event, 1);
      m_isSignal = std::lround(m_target) == m_general_options.m_signal_class;
    }

    std::vector<float> ROOTDataset::getFeature(unsigned int iFeature)
    {
      std::string branchName = Belle2::makeROOTCompatible(m_general_options.m_variables[iFeature]);
      int nentries = getNumberOfEvents();
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
      delete m_tree;
      m_tree = nullptr;
    }

    bool ROOTDataset::checkForBranch(TTree* tree, const std::string& branchname) const
    {
      auto branch = tree->GetListOfBranches()->FindObject(branchname.c_str());
      return branch != nullptr;

    }

    void ROOTDataset::setBranchAddresses()
    {
      if (m_general_options.m_weight_variable == "__weight__") {
        if (checkForBranch(m_tree, "__weight__")) {
          m_tree->SetBranchAddress("__weight__", &m_weight);
        } else {
          B2INFO("Couldn't find default weight feature named __weight__, all weights will be 1. Consider setting the weight variable to an empty string if you don't need it.");
          m_weight = 1;
        }
      } else if (not m_general_options.m_weight_variable.empty()) {
        if (checkForBranch(m_tree, m_general_options.m_weight_variable)) {
          m_tree->SetBranchAddress(m_general_options.m_weight_variable.c_str(), &m_weight);
        } else {
          if (checkForBranch(m_tree, Belle2::makeROOTCompatible(m_general_options.m_weight_variable))) {
            m_tree->SetBranchAddress(Belle2::makeROOTCompatible(m_general_options.m_weight_variable).c_str(), &m_weight);
          } else {
            B2ERROR("Couldn't find given weight variable named " << m_general_options.m_weight_variable <<
                    " (I tried also using makeROOTCompatible)");
            throw std::runtime_error("Couldn't find given weight variable named " + m_general_options.m_weight_variable +
                                     " (I tried also using makeROOTCompatible)");
          }
        }
      }

      if (not m_general_options.m_target_variable.empty()) {
        if (checkForBranch(m_tree, m_general_options.m_target_variable)) {
          m_tree->SetBranchAddress(m_general_options.m_target_variable.c_str(), &m_target);
        } else {
          if (checkForBranch(m_tree, Belle2::makeROOTCompatible(m_general_options.m_target_variable))) {
            m_tree->SetBranchAddress(Belle2::makeROOTCompatible(m_general_options.m_target_variable).c_str(), &m_target);
          } else {
            B2ERROR("Couldn't find given target variable named " << m_general_options.m_target_variable <<
                    " (I tried also using makeROOTCompatible)");
            throw std::runtime_error("Couldn't find given target variable named " + m_general_options.m_target_variable +
                                     " (I tried also using makeROOTCompatible)");
          }
        }
      }

      for (unsigned int i = 0; i < m_general_options.m_variables.size(); ++i)
        if (checkForBranch(m_tree, m_general_options.m_variables[i])) {
          m_tree->SetBranchAddress(m_general_options.m_variables[i].c_str(), &m_input[i]);
        } else {
          if (checkForBranch(m_tree, Belle2::makeROOTCompatible(m_general_options.m_variables[i]))) {
            m_tree->SetBranchAddress(Belle2::makeROOTCompatible(m_general_options.m_variables[i]).c_str(), &m_input[i]);
          } else {
            B2ERROR("Couldn't find given feature variable named " << m_general_options.m_variables[i] <<
                    " (I tried also using makeROOTCompatible)");
            throw std::runtime_error("Couldn't find given feature variable named " + m_general_options.m_variables[i] +
                                     " (I tried also using makeROOTCompatible)");
          }
        }
    }

  }
}
