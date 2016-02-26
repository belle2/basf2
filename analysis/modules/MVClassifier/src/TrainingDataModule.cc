/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/MVClassifier/TrainingDataModule.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <analysis/VariableManager/Utility.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/logging/Logger.h>
#include <analysis/utility/WorkingDirectoryManager.h>
#include <analysis/VariableManager/Utility.h>
#include <framework/logging/Logger.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/pcore/RootMergeable.h>
#include <framework/utilities/Utils.h>

#include <TFile.h>
#include <TTree.h>

#include <memory>

namespace Belle2 {


  REG_MODULE(TrainingData)

  TrainingDataModule::TrainingDataModule() : Module(), m_sample_variable(nullptr)
  {
    setDescription("Trains TMVA method with given particle lists as training samples. "
                   "The target variable has to be an integer valued variable which defines the clusters in the sample. "
                   "e.g. isSignal for signal and background cluster or abs_PDG to define different MC-PDGs as clusters. "
                   "The clusters are trained against each other. "
                   "See also https://belle2.cc.kek.jp/~twiki/bin/view/Software/TMVA for detailed instructions.");

    setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

    std::vector<std::string> empty;
    addParam("listNames", m_listNames,
             "Particles from these ParticleLists are used as input. If no name is given the teacher is applied to every event once, and one can only use variables which accept nullptr as Particle*",
             empty);
    addParam("variables", m_feature_names, "Input variables used by the TMVA method");
    addParam("fileName", m_fileName, "File name used for output file", std::string("training_data.root"));
    addParam("treeName", m_treeName, "Tree name used in output file", std::string("variables"));
    addParam("maxSamples", m_maxSamples, "Maximum number of samples. 0 means no limit.", 0lu);
    addParam("sample", m_sample_name,
             "Variable used for inverse sampling rates. Usually this is the same as the target", std::string(""));
    std::map<int, unsigned int> defaultInverseSamplingRates;
    addParam("inverseSamplingRates", m_inverseSamplingRates, "Map of class id and inverse sampling rate for this class.",
             defaultInverseSamplingRates);
    addParam("lowMemoryProfile", m_lowMemoryProfile,
             "Enables low memory footprint. The internal TTree uses usually O(10MB) of memory, sometimes this is too much, this option tries to reduce the memory footprint of the TMVATeacher module by reducing the basket and cache size of the TTree",
             false);

  }

  void TrainingDataModule::initialize()
  {
    for (auto& name : m_listNames) {
      StoreObjPtr<ParticleList>::required(name);
    }

    if (m_sample_name == "" and not m_inverseSamplingRates.empty()) {
      B2FATAL("Received inverseSamplingRates but no sampling variable was given");
    }

    Variable::Manager& manager = Variable::Manager::Instance();

    m_feature_variables =  manager.getVariables(m_feature_names);
    if (m_feature_variables.size() != m_feature_names.size()) {
      B2FATAL("One or more feature variables could not be loaded via the Variable::Manager. Check the names!");
    }

    if (m_sample_name != "") {
      m_sample_variable = manager.getVariable(m_sample_name);
      if (m_sample_variable == nullptr) {
        B2FATAL("Couldn't find sample variable " << m_sample_name << " via the Variable::Manager. Check the name!");
      }
    } else {
      m_sample_variable = nullptr;
    }
    m_nSamples = 0;

    // Create new tree which stores the input and target variable
    m_feature_input.resize(m_feature_names.size(), 0);

    m_file = TFile::Open(m_fileName.c_str(), "RECREATE");
    m_file->cd();

    // Search for an existing tree in the file
    TTree* tree = new TTree(m_treeName.c_str(), m_treeName.c_str());

    for (unsigned int i = 0; i < m_feature_names.size(); ++i)
      tree->Branch(Variable::makeROOTCompatible(m_feature_names[i]).c_str(), &m_feature_input[i]);
    tree->Branch("__weight__", &m_original_weight);

    m_tree.registerInDataStore(m_treeName, DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);
    m_tree.construct();
    m_tree->assign(tree);

    if (m_lowMemoryProfile) {
      m_tree->get().SetBasketSize("*", 1600);
      m_tree->get().SetCacheSize(100000);
    }

  }

  void TrainingDataModule::terminate()
  {
    if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
      m_file->cd();
      m_tree->get().Write("", TObject::kOverwrite);

      const bool writeError = m_file->TestBit(TFile::kWriteError);
      if (writeError) {
        //m_file deleted first so we have a chance of closing it (though that will probably fail)
        delete m_file;
        B2FATAL("A write error occured while saving '" << m_fileName  << "', please check if enough disk space is available.");
      }

      m_tree->get().SetDirectory(nullptr);
    }

    m_tree->get().SetDirectory(nullptr);
    m_file->Close();

  }

  float TrainingDataModule::getInverseSamplingRateWeight(const Particle* particle)
  {
    m_nSamples++;

    if (m_maxSamples != 0 and m_nSamples > m_maxSamples) {
      return 0.0;
    }

    if (m_sample_variable == nullptr)
      return 1.0;

    long target = std::lround(m_sample_variable->function(particle));
    if (m_inverseSamplingRates.find(target) != m_inverseSamplingRates.end()) {
      if (m_iSamples.find(target) == m_iSamples.end()) {
        m_iSamples[target] = 0;
      }
      m_iSamples[target]++;
      if (m_iSamples[target] % m_inverseSamplingRates[target] != 0)
        return 0;
      else {
        m_iSamples[target] = 0;
        return m_inverseSamplingRates[target];
      }
    }
    return 1.0;
  }

  void TrainingDataModule::addSample(const Particle* particle, float weight)
  {
    // Fill the tree with the input variables
    m_original_weight = weight;
    for (unsigned int i = 0; i < m_feature_variables.size(); ++i) {
      if (m_feature_variables[i] != nullptr)
        m_feature_input[i] = m_feature_variables[i]->function(particle);
      if (!std::isfinite(m_feature_input[i])) {
        B2ERROR("Output of variable " << m_feature_variables[i]->name << " is " << m_feature_input[i] <<
                ", please fix it. Candidate will be skipped.");
        return;
      }
    }
    m_tree->get().Fill();
  }


  void TrainingDataModule::event()
  {
    for (auto& listName : m_listNames) {
      StoreObjPtr<ParticleList> list(listName);
      // Calculate Signal Probability for Particles
      for (unsigned i = 0; i < list->getListSize(); ++i) {
        const Particle* particle = list->getParticle(i);
        float weight = getInverseSamplingRateWeight(particle);
        if (weight > 0)
          addSample(particle, weight);
      }
    }

    if (m_listNames.empty()) {
      float weight = getInverseSamplingRateWeight(nullptr);
      if (weight > 0)
        addSample(nullptr, weight);
    }
  }

} // Belle2 namespace

