/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <mva/interface/Dataset.h>

#include <framework/utilities/MakeROOTCompatible.h>
#include <framework/logging/Logger.h>
#include <framework/io/RootIOUtilities.h>

#include <TLeaf.h>

#include <boost/filesystem/operations.hpp>

namespace Belle2 {
  namespace MVA {

    Dataset::Dataset(const GeneralOptions& general_options) : m_general_options(general_options)
    {
      m_input.resize(m_general_options.m_variables.size(), 0);
      m_spectators.resize(m_general_options.m_spectators.size(), 0);
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

    unsigned int Dataset::getFeatureIndex(const std::string& feature)
    {

      auto it = std::find(m_general_options.m_variables.begin(), m_general_options.m_variables.end(), feature);
      if (it == m_general_options.m_variables.end()) {
        B2ERROR("Unknown feature named " << feature);
        return 0;
      }
      return std::distance(m_general_options.m_variables.begin(), it);

    }

    unsigned int Dataset::getSpectatorIndex(const std::string& spectator)
    {

      auto it = std::find(m_general_options.m_spectators.begin(), m_general_options.m_spectators.end(), spectator);
      if (it == m_general_options.m_spectators.end()) {
        B2ERROR("Unknown spectator named " << spectator);
        return 0;
      }
      return std::distance(m_general_options.m_spectators.begin(), it);

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

    std::vector<float> Dataset::getSpectator(unsigned int iSpectator)
    {

      std::vector<float> result(getNumberOfEvents());
      for (unsigned int iEvent = 0; iEvent < getNumberOfEvents(); ++iEvent) {
        loadEvent(iEvent);
        result[iEvent] = m_spectators[iSpectator];
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


    SingleDataset::SingleDataset(const GeneralOptions& general_options, const std::vector<float>& input, float target,
                                 const std::vector<float>& spectators) : Dataset(general_options)
    {
      m_input = input;
      m_spectators = spectators;
      m_target = target;
      m_weight = 1.0;
      m_isSignal = std::lround(target) == m_general_options.m_signal_class;
    }

    MultiDataset::MultiDataset(const GeneralOptions& general_options, const std::vector<std::vector<float>>& input,
                               const std::vector<std::vector<float>>& spectators,
                               const std::vector<float>& targets, const std::vector<float>& weights) : Dataset(general_options),  m_matrix(input),
      m_spectator_matrix(spectators),
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
      if (m_spectator_matrix.size() > 0 and m_matrix.size() != m_spectator_matrix.size()) {
        B2ERROR("Feature matrix and spectator matrix need same number of elements in MultiDataset, got " << m_spectator_matrix.size() <<
                " and " <<
                m_matrix.size());
      }
    }


    void MultiDataset::loadEvent(unsigned int iEvent)
    {
      m_input = m_matrix[iEvent];

      if (m_spectator_matrix.size() > 0) {
        m_spectators = m_spectator_matrix[iEvent];
      }

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

      for (auto& v : m_general_options.m_spectators) {
        auto it = std::find(m_dataset.m_general_options.m_spectators.begin(), m_dataset.m_general_options.m_spectators.end(), v);
        if (it == m_dataset.m_general_options.m_spectators.end()) {
          B2ERROR("Couldn't find spectator " << v << " in GeneralOptions");
          throw std::runtime_error("Couldn't find spectator " + v + " in GeneralOptions");
        }
        m_spectator_indices.push_back(it - m_dataset.m_general_options.m_spectators.begin());
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

      for (unsigned int iSpectator = 0; iSpectator < m_spectators.size(); ++iSpectator) {
        m_spectators[iSpectator] = m_dataset.m_spectators[m_spectator_indices[iSpectator]];
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

    std::vector<float> SubDataset::getSpectator(unsigned int iSpectator)
    {

      auto v = m_dataset.getSpectator(m_spectator_indices[iSpectator]);
      if (not m_use_event_indices)
        return v;
      std::vector<float> result(m_event_indices.size());
      for (unsigned int iEvent = 0; iEvent < getNumberOfEvents(); ++iEvent) {
        result[iEvent] = v[m_event_indices[iEvent]];
      }
      return result;

    }

    CombinedDataset::CombinedDataset(const GeneralOptions& general_options, Dataset& signal_dataset,
                                     Dataset& background_dataset) : Dataset(general_options), m_signal_dataset(signal_dataset),
      m_background_dataset(background_dataset) { }

    void CombinedDataset::loadEvent(unsigned int iEvent)
    {
      if (iEvent < m_signal_dataset.getNumberOfEvents()) {
        m_signal_dataset.loadEvent(iEvent);
        m_target = 1.0;
        m_isSignal = true;
        m_weight = m_signal_dataset.m_weight;
        m_input = m_signal_dataset.m_input;
        m_spectators = m_signal_dataset.m_spectators;
      } else {
        m_background_dataset.loadEvent(iEvent - m_signal_dataset.getNumberOfEvents());
        m_target = 0.0;
        m_isSignal = false;
        m_weight = m_background_dataset.m_weight;
        m_input = m_background_dataset.m_input;
        m_spectators = m_background_dataset.m_spectators;
      }
    }

    std::vector<float> CombinedDataset::getFeature(unsigned int iFeature)
    {

      auto s = m_signal_dataset.getFeature(iFeature);
      auto b = m_background_dataset.getFeature(iFeature);
      s.insert(s.end(), b.begin(), b.end());
      return s;

    }

    std::vector<float> CombinedDataset::getSpectator(unsigned int iSpectator)
    {

      auto s = m_signal_dataset.getSpectator(iSpectator);
      auto b = m_background_dataset.getSpectator(iSpectator);
      s.insert(s.end(), b.begin(), b.end());
      return s;

    }

    ROOTDataset::ROOTDataset(const GeneralOptions& general_options) : Dataset(general_options)
    {
      m_input_double.resize(m_general_options.m_variables.size(), 0);
      m_spectators_double.resize(m_general_options.m_spectators.size(), 0);
      m_target_double = 0.0;
      m_weight_double = 1.0;

      for (const auto& variable : general_options.m_variables)
        for (const auto& spectator : general_options.m_spectators)
          if (variable == spectator or variable == general_options.m_target_variable or spectator == general_options.m_target_variable) {
            B2ERROR("Interface doesn't support variable more then one time in either spectators, variables or target variable");
            throw std::runtime_error("Interface doesn't support variable more then one time in either spectators, variables or target variable");
          }

      std::vector<std::string> filenames;
      for (const auto& filename : m_general_options.m_datafiles) {
        if (boost::filesystem::exists(filename)) {
          filenames.push_back(filename);
        } else {
          auto temp = RootIOUtilities::expandWordExpansions(m_general_options.m_datafiles);
          filenames.insert(filenames.end(), temp.begin(), temp.end());
        }
      }
      if (filenames.empty()) {
        B2ERROR("Found no valid filenames in GeneralOptions");
        throw std::runtime_error("Found no valid filenames in GeneralOptions");
      }

      //Open TFile
      TDirectory* dir = gDirectory;
      for (const auto& filename : filenames) {
        if (not boost::filesystem::exists(filename)) {
          B2ERROR("Error given ROOT file does not exist " << filename);
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
          B2ERROR("Error during open of ROOT file named " << filename << " cannot retrieve tree named " <<
                  m_general_options.m_treename);
          throw std::runtime_error("Error during open of ROOT file named " + filename + " cannot retrieve tree named " +
                                   m_general_options.m_treename);
        }
      }
      setRootInputType();
      setBranchAddresses();
    }

    void ROOTDataset::loadEvent(unsigned int event)
    {
      if (m_tree->GetEntry(event, 0) == 0) {
        B2ERROR("Error during loading entry from chain");
      }
      if (m_isDoubleInputType) {
        m_weight = (float) m_weight_double;
        m_target = (float) m_target_double;
        for (unsigned int i = 0; i < m_input_double.size(); i++)
          m_input[i] = (float) m_input_double[i];
        for (unsigned int i = 0; i < m_spectators_double.size(); i++)
          m_spectators[i] = (float) m_spectators_double[i];
      }

      m_isSignal = std::lround(m_target) == m_general_options.m_signal_class;
    }

    std::vector<float> ROOTDataset::getWeights()
    {
      std::string branchName = Belle2::makeROOTCompatible(m_general_options.m_weight_variable);
      if (branchName.empty()) {
        B2INFO("No TBranch name given for weights. Using 1s as default weights.");
        int nentries = getNumberOfEvents();
        std::vector<float> values(nentries, 1.);
        return values;
      }
      if (branchName == "__weight__") {
        if (!checkForBranch(m_tree, "__weight__")) {
          B2INFO("No default weight branch with name __weight__ found. Using 1s as default weights.");
          int nentries = getNumberOfEvents();
          std::vector<float> values(nentries, 1.);
          return values;
        }
      }

      std::string typeName = "weights";

      if (m_isDoubleInputType)
        return ROOTDataset::getVectorFromTTree(typeName, branchName, m_weight_double);

      return ROOTDataset::getVectorFromTTree(typeName, branchName, m_weight);
    }

    std::vector<float> ROOTDataset::getFeature(unsigned int iFeature)
    {
      if (iFeature >= getNumberOfFeatures()) {
        B2ERROR("Feature index " << iFeature << " is out of bounds of given number of features: "
                << getNumberOfFeatures());
      }

      std::string branchName = Belle2::makeROOTCompatible(m_general_options.m_variables[iFeature]);
      std::string typeName = "features";

      if (m_isDoubleInputType)
        return ROOTDataset::getVectorFromTTree(typeName, branchName, m_input_double[iFeature]);

      return ROOTDataset::getVectorFromTTree(typeName, branchName, m_input[iFeature]);
    }

    std::vector<float> ROOTDataset::getSpectator(unsigned int iSpectator)
    {
      if (iSpectator >= getNumberOfSpectators()) {
        B2ERROR("Spectator index " << iSpectator << " is out of bounds of given number of spectators: "
                << getNumberOfSpectators());
      }

      std::string branchName = Belle2::makeROOTCompatible(m_general_options.m_spectators[iSpectator]);
      std::string typeName = "spectators";

      if (m_isDoubleInputType)
        return ROOTDataset::getVectorFromTTree(typeName, branchName, m_spectators_double[iSpectator]);

      return ROOTDataset::getVectorFromTTree(typeName, branchName, m_spectators[iSpectator]);
    }

    ROOTDataset::~ROOTDataset()
    {
      delete m_tree;
      m_tree = nullptr;
    }

    template<class T>
    std::vector<float> ROOTDataset::getVectorFromTTree(std::string& variableType, std::string& branchName,
                                                       T& memberVariableTarget)
    {
      int nentries = getNumberOfEvents();
      std::vector<float> values(nentries);

      // Float or Double to be filled
      T object;
      auto currentTreeNumber = m_tree->GetTreeNumber();
      TBranch* branch = m_tree->GetBranch(branchName.c_str());
      if (not branch) {
        B2ERROR("TBranch for " + variableType + " named '" << branchName.c_str()  << "' does not exist!");
      }
      branch->SetAddress(&object);
      for (int i = 0; i < nentries; ++i) {
        auto entry = m_tree->LoadTree(i);
        if (entry < 0) {
          B2ERROR("Error during loading root tree from chain, error code: " << entry);
        }
        // if current tree changed we have to update the branch
        if (currentTreeNumber != m_tree->GetTreeNumber()) {
          currentTreeNumber = m_tree->GetTreeNumber();
          branch = m_tree->GetBranch(branchName.c_str());
          branch->SetAddress(&object);
        }
        branch->GetEntry(entry);
        values[i] = object;
      }
      // Reset branch to correct input address, just to be sure
      m_tree->SetBranchAddress(branchName.c_str(), &memberVariableTarget);
      return values;
    }

    bool ROOTDataset::checkForBranch(TTree* tree, const std::string& branchname) const
    {
      auto branch = tree->GetListOfBranches()->FindObject(branchname.c_str());
      return branch != nullptr;

    }

    template<class T>
    void ROOTDataset::setScalarVariableAddress(std::string& variableType, std::string& variableName,
                                               T& variableTarget)
    {
      if (not variableName.empty()) {
        if (checkForBranch(m_tree, variableName)) {
          m_tree->SetBranchStatus(variableName.c_str(), true);
          m_tree->SetBranchAddress(variableName.c_str(), &variableTarget);
        } else {
          if (checkForBranch(m_tree, Belle2::makeROOTCompatible(variableName))) {
            m_tree->SetBranchStatus(Belle2::makeROOTCompatible(variableName).c_str(), true);
            m_tree->SetBranchAddress(Belle2::makeROOTCompatible(variableName).c_str(), &variableTarget);
          } else {
            B2ERROR("Couldn't find given " << variableType << " variable named " << variableName <<
                    " (I tried also using makeROOTCompatible)");
            throw std::runtime_error("Couldn't find given " + variableType + " variable named " + variableName +
                                     " (I tried also using makeROOTCompatible)");
          }
        }
      }
    }

    template<class T>
    void ROOTDataset::setVectorVariableAddress(std::string& variableType, std::vector<std::string>& variableNames,
                                               T& variableTargets)
    {
      for (unsigned int i = 0; i < variableNames.size(); ++i)
        ROOTDataset::setScalarVariableAddress(variableType, variableNames[i], variableTargets[i]);
    }

    void ROOTDataset::setBranchAddresses()
    {
      // Deactivate all branches by default
      m_tree->SetBranchStatus("*", false);
      std::string typeName;

      if (m_general_options.m_weight_variable.empty()) {
        m_weight = 1;
        m_weight_double = 1;
        B2INFO("No weight variable provided. The weight will be set to 1.");
      }

      if (m_general_options.m_weight_variable == "__weight__") {
        if (checkForBranch(m_tree, "__weight__")) {
          m_tree->SetBranchStatus("__weight__", true);
          if (m_isDoubleInputType)
            m_tree->SetBranchAddress("__weight__", &m_weight_double);
          else
            m_tree->SetBranchAddress("__weight__", &m_weight);
        } else {
          B2INFO("Couldn't find default weight feature named __weight__, all weights will be 1. Consider setting the "
                 "weight variable to an empty string if you don't need it.");
          m_weight = 1;
          m_weight_double = 1;
        }
      } else if (m_isDoubleInputType) {
        typeName = "weight";
        ROOTDataset::setScalarVariableAddress(typeName, m_general_options.m_weight_variable, m_weight_double);
      } else {
        typeName = "weight";
        ROOTDataset::setScalarVariableAddress(typeName, m_general_options.m_weight_variable, m_weight);
      }

      if (m_isDoubleInputType) {
        typeName = "target";
        ROOTDataset::setScalarVariableAddress(typeName, m_general_options.m_target_variable, m_target_double);
        typeName = "feature";
        ROOTDataset::setVectorVariableAddress(typeName, m_general_options.m_variables, m_input_double);
        typeName = "spectator";
        ROOTDataset::setVectorVariableAddress(typeName, m_general_options.m_spectators, m_spectators_double);
      } else {
        typeName = "target";
        ROOTDataset::setScalarVariableAddress(typeName, m_general_options.m_target_variable, m_target);
        typeName = "feature";
        ROOTDataset::setVectorVariableAddress(typeName, m_general_options.m_variables, m_input);
        typeName = "spectator";
        ROOTDataset::setVectorVariableAddress(typeName, m_general_options.m_spectators, m_spectators);
      }
    }


    void ROOTDataset::setRootInputType()
    {
      std::string control_variable;
      for (auto& variable : m_general_options.m_variables) {
        if (checkForBranch(m_tree, variable))
          control_variable = variable;
        else if (checkForBranch(m_tree, Belle2::makeROOTCompatible(variable)))
          control_variable = Belle2::makeROOTCompatible(variable);
        if (not control_variable.empty()) {
          TBranch* branch = m_tree->GetBranch(control_variable.c_str());
          TLeaf* leaf = branch->GetLeaf(control_variable.c_str());
          std::string type_name = leaf->GetTypeName();
          if (type_name == "Double_t")
            m_isDoubleInputType = true;
          else if (type_name == "Float_t")
            m_isDoubleInputType = false;
          else {
            B2FATAL("Unknown root input type: " << type_name);
            throw std::runtime_error("Unknown root input type: " + type_name);
          }
          return;
        }
      }
      B2FATAL("No valid feature was found. Check your input features.");
      throw std::runtime_error("No valid feature was found. Check your input features.");
    }


  }
}
