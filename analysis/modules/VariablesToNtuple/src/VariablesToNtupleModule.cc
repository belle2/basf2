/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/VariablesToNtuple/VariablesToNtupleModule.h>

// analysis
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>

// framework
#include <framework/logging/Logger.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/core/ModuleParam.templateDetails.h>

// framework - root utilities
#include <framework/utilities/MakeROOTCompatible.h>
#include <framework/utilities/RootFileCreationManager.h>

#include <cmath>

using namespace std;
using namespace Belle2;

// Register module in the framework
REG_MODULE(VariablesToNtuple)


VariablesToNtupleModule::VariablesToNtupleModule() :
  Module(), m_tree("", DataStore::c_Persistent)
{
  //Set module properties
  setDescription("Calculate variables specified by the user for a given ParticleList and save them into a TNtuple. The TNtuple is candidate-based, meaning that the variables of each candidate are saved separate rows.");
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

  vector<string> emptylist;
  addParam("particleList", m_particleList,
           "Name of particle list with reconstructed particles. If no list is provided the variables are saved once per event (only possible for event-type variables)",
           std::string(""));
  addParam("variables", m_variables,
           "List of variables (or collections) to save. Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector.",
           emptylist);

  addParam("fileName", m_fileName, "Name of ROOT file for output.", string("VariablesToNtuple.root"));
  addParam("treeName", m_treeName, "Name of the NTuple in the saved file.", string("ntuple"));

  std::tuple<std::string, std::map<int, unsigned int>> default_sampling{"", {}};
  addParam("sampling", m_sampling,
           "Tuple of variable name and a map of integer values and inverse sampling rate. E.g. (signal, {1: 0, 0:10}) selects all signal candidates and every 10th background candidate.",
           default_sampling);
}

void VariablesToNtupleModule::initialize()
{
  m_eventMetaData.isRequired();
  if (not m_particleList.empty())
    StoreObjPtr<ParticleList>().isRequired(m_particleList);


  // Initializing the output root file
  if (m_fileName.empty()) {
    B2FATAL("Output root file name is not set. Please set a vaild root output file name (\"fileName\" module parameter).");
  }
  // See if there is already a file in which case add a new tree to it ...
  // otherwise create a new file (all handled by framework)
  m_file =  RootFileCreationManager::getInstance().getFile(m_fileName);
  if (!m_file) {
    B2ERROR("Could not create file \"" << m_fileName <<
            "\". Please set a vaild root output file name (\"fileName\" module parameter).");
    return;
  }

  TDirectory::TContext directoryGuard(m_file.get());

  // check if TTree with that name already exists
  if (m_file->Get(m_treeName.c_str())) {
    B2FATAL("Tree with the name \"" << m_treeName
            << "\" already exists in the file \"" << m_fileName << "\"\n"
            << "\nYou probably want to either set the output fileName or the treeName to something else:\n\n"
            << "   from modularAnalysis import variablesToNtuple\n"
            << "   variablesToNtuple('pi+:all', ['p'], treename='pions', filename='variablesToNtuple.root')\n"
            << "   variablesToNtuple('gamma:all', ['p'], treename='photons', filename='variablesToNtuple.root') # two trees, same file\n"
            << "\n == Or ==\n"
            << "   from modularAnalysis import variablesToNtuple\n"
            << "   variablesToNtuple('pi+:all', ['p'], filename='pions.root')\n"
            << "   variablesToNtuple('gamma:all', ['p'], filename='photons.root') # two files\n"
           );
    return;
  }

  // set up tree and register it in the datastore
  m_tree.registerInDataStore(m_fileName + m_treeName, DataStore::c_DontWriteOut);
  m_tree.construct(m_treeName.c_str(), "");
  m_tree->get().SetCacheSize(100000);

  // declare counter branches - pass through variable list, remove counters added by user
  m_tree->get().Branch("__experiment__", &m_experiment, "__experiment__/I");
  m_tree->get().Branch("__run__", &m_run, "__run__/I");
  m_tree->get().Branch("__event__", &m_event, "__event__/I");
  m_tree->get().Branch("__production__", &m_production, "__production__/I");
  if (not m_particleList.empty()) {
    m_tree->get().Branch("__candidate__", &m_candidate, "__candidate__/I");
    m_tree->get().Branch("__ncandidates__", &m_ncandidates, "__ncandidates__/I");
  }
  for (const auto& variable : m_variables)
    if (Variable::isCounterVariable(variable)) {
      B2WARNING("The counter '" << variable
                << "' is handled automatically by VariablesToNtuple, you don't need to add it.");
    }

  // declare branches and get the variable strings
  m_variables = Variable::Manager::Instance().resolveCollections(m_variables);
  m_branchAddresses.resize(m_variables.size() + 1);
  m_tree->get().Branch("__weight__", &m_branchAddresses[0], "__weight__/D");
  size_t enumerate = 1;
  for (const string& varStr : m_variables) {
    string branchName = makeROOTCompatible(varStr);

    // Check for deprecated variables
    Variable::Manager::Instance().checkDeprecatedVariable(varStr);

    m_tree->get().Branch(branchName.c_str(), &m_branchAddresses[enumerate], (branchName + "/D").c_str());

    // also collection function pointers
    const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(varStr);
    if (!var) {
      B2ERROR("Variable '" << varStr << "' is not available in Variable::Manager!");
    } else {
      if (m_particleList.empty() && var->description.find("[Eventbased]") == string::npos) {
        B2ERROR("Variable '" << varStr << "' is not an event-based variable, "
                "but you are using VariablesToNtuple without a decay string, i.e. in the event-wise mode.\n"
                "If you have created an event-based alias you can wrap your alias with `eventCached` to "
                "declare it as event based, which avoids this error.\n\n"
                "vm.addAlias('myAliasName', 'eventCached(myAlias)')");
        continue;
      }
      m_functions.push_back(var->function);
    }
    enumerate++;
  }
  m_tree->get().SetBasketSize("*", 1600);

  m_sampling_name = std::get<0>(m_sampling);
  m_sampling_rates = std::get<1>(m_sampling);

  if (m_sampling_name != "") {
    m_sampling_variable = Variable::Manager::Instance().getVariable(m_sampling_name);
    if (m_sampling_variable == nullptr) {
      B2FATAL("Couldn't find sample variable " << m_sampling_name << " via the Variable::Manager. Check the name!");
    }
    for (const auto& pair : m_sampling_rates)
      m_sampling_counts[pair.first] = 0;
  } else {
    m_sampling_variable = nullptr;
  }
}


float VariablesToNtupleModule::getInverseSamplingRateWeight(const Particle* particle)
{
  if (m_sampling_variable == nullptr)
    return 1.0;

  long target = std::lround(m_sampling_variable->function(particle));
  if (m_sampling_rates.find(target) != m_sampling_rates.end() and m_sampling_rates[target] > 0) {
    m_sampling_counts[target]++;
    if (m_sampling_counts[target] % m_sampling_rates[target] != 0)
      return 0;
    else {
      m_sampling_counts[target] = 0;
      return m_sampling_rates[target];
    }
  }
  return 1.0;
}

void VariablesToNtupleModule::event()
{
  m_event = m_eventMetaData->getEvent();
  m_run = m_eventMetaData->getRun();
  m_experiment = m_eventMetaData->getExperiment();
  m_production = m_eventMetaData->getProduction();

  if (m_particleList.empty()) {
    m_branchAddresses[0] = getInverseSamplingRateWeight(nullptr);
    if (m_branchAddresses[0] > 0) {
      for (unsigned int iVar = 0; iVar < m_variables.size(); iVar++) {
        m_branchAddresses[iVar + 1] = m_functions[iVar](nullptr);
      }
      m_tree->get().Fill();
    }

  } else {
    StoreObjPtr<ParticleList> particlelist(m_particleList);
    m_ncandidates = particlelist->getListSize();
    for (unsigned int iPart = 0; iPart < m_ncandidates; iPart++) {
      m_candidate = iPart;
      const Particle* particle = particlelist->getParticle(iPart);
      m_branchAddresses[0] = getInverseSamplingRateWeight(particle);
      if (m_branchAddresses[0] > 0) {
        for (unsigned int iVar = 0; iVar < m_variables.size(); iVar++) {
          m_branchAddresses[iVar + 1] = m_functions[iVar](particle);
        }
        m_tree->get().Fill();
      }
    }
  }
}

void VariablesToNtupleModule::terminate()
{
  if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
    B2INFO("Writing NTuple " << m_treeName);
    TDirectory::TContext directoryGuard(m_file.get());
    m_tree->write(m_file.get());

    const bool writeError = m_file->TestBit(TFile::kWriteError);
    m_file.reset();
    if (writeError) {
      B2FATAL("A write error occured while saving '" << m_fileName  << "', please check if enough disk space is available.");
    }
  }
}
