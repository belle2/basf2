/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/VariablesToEventBasedTree/VariablesToEventBasedTreeModule.h>

// analysis
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/StringWrapper.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>

// framework
#include <framework/logging/Logger.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/utilities/MakeROOTCompatible.h>
#include <framework/utilities/RootFileCreationManager.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/core/Environment.h>

#include <cmath>

using namespace std;
using namespace Belle2;

// Register module in the framework
REG_MODULE(VariablesToEventBasedTree);


VariablesToEventBasedTreeModule::VariablesToEventBasedTreeModule() :
  Module(), m_tree("", DataStore::c_Persistent)
{
  //Set module properties
  setDescription("Calculate variables specified by the user for a given ParticleList and save them into a TTree. The Tree is event-based, meaning that the variables of each candidate for each event are saved in an array of a branch of the Tree.");
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

  vector<string> emptylist;
  addParam("particleList", m_particleList,
           "Name of particle list with reconstructed particles. An empty ParticleList is not supported. Use the VariablesToNtupleModule for this use-case",
           std::string(""));
  addParam("variables", m_variables,
           "List of variables (or collections) to save for each candidate. Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector.",
           emptylist);

  addParam("event_variables", m_event_variables,
           "List of variables (or collections) to save for each event. Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector. Only event-based variables are allowed here.",
           emptylist);

  addParam("fileName", m_fileName, "Name of ROOT file for output. Can be overridden using the -o argument of basf2.",
           string("VariablesToEventBasedTree.root"));
  addParam("treeName", m_treeName, "Name of the NTuple in the saved file.", string("tree"));
  addParam("maxCandidates", m_maxCandidates, "The maximum number of candidates in the ParticleList per entry of the Tree.", 100u);

  std::tuple<std::string, std::map<int, unsigned int>> default_sampling{"", {}};
  addParam("sampling", m_sampling,
           "Tuple of variable name and a map of integer values and inverse sampling rate. E.g. (signal, {1: 0, 0:10}) selects all signal events and every 10th background event. Variable must be event-based.",
           default_sampling);

  addParam("fileNameSuffix", m_fileNameSuffix, "The suffix of the output ROOT file to be appended before ``.root``.",
           string(""));

  addParam("storeEventType", m_storeEventType,
           "If true, the branch __eventType__ is added. The eventType information is available from MC16 on.", true);
}

void VariablesToEventBasedTreeModule::initialize()
{
  m_eventMetaData.isRequired();
  StoreObjPtr<ParticleList>().isRequired(m_particleList);

  // override the output file name with what's been provided with the -o option
  const std::string& outputFileArgument = Environment::Instance().getOutputFileOverride();
  if (!outputFileArgument.empty())
    m_fileName = outputFileArgument;

  if (!m_fileNameSuffix.empty())
    m_fileName = m_fileName.insert(m_fileName.rfind(".root"), m_fileNameSuffix);

  // See if there is already a file in which case add a new tree to it ...
  // otherwise create a new file (all handled by framework)
  m_file =  RootFileCreationManager::getInstance().getFile(m_fileName);
  if (!m_file) {
    B2ERROR("Could not create file \"" << m_fileName <<
            "\". Please set a valid root output file name (\"fileName\" module parameter).");
    return;
  }

  m_file->cd();

  // check if TTree with that name already exists
  if (m_file->Get(m_treeName.c_str())) {
    B2FATAL("Tree with the name " << m_treeName << " already exists in the file " << m_fileName);
    return;
  }

  m_variables = Variable::Manager::Instance().resolveCollections(m_variables);
  // remove duplicates from list of variables but keep the previous order
  unordered_set<string> seen;
  auto newEnd = remove_if(m_variables.begin(), m_variables.end(), [&seen](const string & varStr) {
    if (seen.find(varStr) != std::end(seen)) return true;
    seen.insert(varStr);
    return false;
  });
  m_variables.erase(newEnd, m_variables.end());

  m_event_variables = Variable::Manager::Instance().resolveCollections(m_event_variables);
  // remove duplicates from list of variables but keep the previous order
  unordered_set<string> seenEventVariables;
  auto eventVariablesEnd = remove_if(m_event_variables.begin(),
  m_event_variables.end(), [&seenEventVariables](const string & varStr) {
    if (seenEventVariables.find(varStr) != std::end(seenEventVariables)) return true;
    seenEventVariables.insert(varStr);
    return false;
  });
  m_event_variables.erase(eventVariablesEnd, m_event_variables.end());

  m_tree.registerInDataStore(m_fileName + m_treeName, DataStore::c_DontWriteOut);
  m_tree.construct(m_treeName.c_str(), "");

  m_valuesDouble.resize(m_variables.size());
  m_valuesInt.resize(m_variables.size());
  m_event_valuesDouble.resize(m_event_variables.size());
  m_event_valuesInt.resize(m_event_variables.size());

  m_tree->get().Branch("__event__", &m_event, "__event__/i");
  m_tree->get().Branch("__run__", &m_run, "__run__/I");
  m_tree->get().Branch("__experiment__", &m_experiment, "__experiment__/I");
  m_tree->get().Branch("__production__", &m_production, "__production__/I");
  m_tree->get().Branch("__ncandidates__", &m_ncandidates, "__ncandidates__/I");
  m_tree->get().Branch("__weight__", &m_weight, "__weight__/F");

  if (m_stringWrapper.isOptional("MCDecayString"))
    m_tree->get().Branch("__MCDecayString__", &m_MCDecayString);

  if (m_storeEventType) {
    m_tree->get().Branch("__eventType__", &m_eventType);
    if (not m_eventExtraInfo.isOptional())
      B2INFO("EventExtraInfo is not registered. __eventType__ will be empty. The eventType is available from MC16 on.");
  }


  for (unsigned int i = 0; i < m_event_variables.size(); ++i) {
    auto varStr = m_event_variables[i];

    if (Variable::isCounterVariable(varStr)) {
      B2WARNING("The counter '" << varStr
                << "' is handled automatically by VariablesToEventBasedTree, you don't need to add it.");
      continue;
    }

    //also collection function pointers
    const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(varStr);
    if (!var) {
      B2ERROR("Variable '" << varStr << "' is not available in Variable::Manager!");
    } else {
      if (var->variabletype == Variable::Manager::VariableDataType::c_double) {
        m_tree->get().Branch(MakeROOTCompatible::makeROOTCompatible(varStr).c_str(), &m_event_valuesDouble[i],
                             (MakeROOTCompatible::makeROOTCompatible(varStr) + "/D").c_str());
      } else if (var->variabletype == Variable::Manager::VariableDataType::c_int) {
        m_tree->get().Branch(MakeROOTCompatible::makeROOTCompatible(varStr).c_str(), &m_event_valuesInt[i],
                             (MakeROOTCompatible::makeROOTCompatible(varStr) + "/I").c_str());
      } else if (var->variabletype == Variable::Manager::VariableDataType::c_bool) {
        m_tree->get().Branch(MakeROOTCompatible::makeROOTCompatible(varStr).c_str(), &m_event_valuesInt[i],
                             (MakeROOTCompatible::makeROOTCompatible(varStr) + "/O").c_str());
      }
      m_event_functions.push_back(var->function);
    }
  }

  for (unsigned int i = 0; i < m_variables.size(); ++i) {
    auto varStr = m_variables[i];
    m_valuesDouble[i].resize(m_maxCandidates);
    m_valuesInt[i].resize(m_maxCandidates);

    //also collection function pointers
    const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(varStr);
    if (!var) {
      B2ERROR("Variable '" << varStr << "' is not available in Variable::Manager!");
    } else {
      if (var->variabletype == Variable::Manager::VariableDataType::c_double) {
        m_tree->get().Branch(MakeROOTCompatible::makeROOTCompatible(varStr).c_str(), &m_valuesDouble[i][0],
                             (MakeROOTCompatible::makeROOTCompatible(varStr) + "[__ncandidates__]/D").c_str());
      } else if (var->variabletype == Variable::Manager::VariableDataType::c_int) {
        m_tree->get().Branch(MakeROOTCompatible::makeROOTCompatible(varStr).c_str(), &m_valuesInt[i][0],
                             (MakeROOTCompatible::makeROOTCompatible(varStr) + "[__ncandidates__]/I").c_str());
      } else if (var->variabletype == Variable::Manager::VariableDataType::c_bool) {
        m_tree->get().Branch(MakeROOTCompatible::makeROOTCompatible(varStr).c_str(), &m_valuesInt[i][0],
                             (MakeROOTCompatible::makeROOTCompatible(varStr) + "[__ncandidates__]/O").c_str());
      }
      m_functions.push_back(var->function);
    }
  }

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


float VariablesToEventBasedTreeModule::getInverseSamplingRateWeight()
{

  if (m_sampling_variable == nullptr)
    return 1.0;

  long target = 0;
  if (m_sampling_variable->variabletype == Variable::Manager::VariableDataType::c_double) {
    target = std::lround(std::get<double>(m_sampling_variable->function(nullptr)));
  } else if (m_sampling_variable->variabletype == Variable::Manager::VariableDataType::c_int) {
    target = std::lround(std::get<int>(m_sampling_variable->function(nullptr)));
  } else if (m_sampling_variable->variabletype == Variable::Manager::VariableDataType::c_bool) {
    target = std::lround(std::get<bool>(m_sampling_variable->function(nullptr)));
  }

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

void VariablesToEventBasedTreeModule::event()
{
  // get counter numbers
  m_event = m_eventMetaData->getEvent();
  m_run = m_eventMetaData->getRun();
  m_experiment = m_eventMetaData->getExperiment();
  m_production = m_eventMetaData->getProduction();

  if (m_stringWrapper.isValid())
    m_MCDecayString = m_stringWrapper->getString();
  else
    m_MCDecayString = "";

  if (m_storeEventType and m_eventExtraInfo.isValid())
    m_eventType = m_eventExtraInfo->getEventType();
  else
    m_eventType = "";

  StoreObjPtr<ParticleList> particlelist(m_particleList);
  m_ncandidates = particlelist->getListSize();
  m_weight = getInverseSamplingRateWeight();
  if (m_weight > 0) {
    for (unsigned int iVar = 0; iVar < m_event_functions.size(); iVar++) {
      if (std::holds_alternative<double>(m_event_functions[iVar](nullptr))) {
        m_event_valuesDouble[iVar] = std::get<double>(m_event_functions[iVar](nullptr));
      } else if (std::holds_alternative<int>(m_event_functions[iVar](nullptr))) {
        m_event_valuesInt[iVar] = std::get<int>(m_event_functions[iVar](nullptr));
      } else if (std::holds_alternative<bool>(m_event_functions[iVar](nullptr))) {
        m_event_valuesInt[iVar] = std::get<bool>(m_event_functions[iVar](nullptr));
      }
    }
    for (unsigned int iPart = 0; iPart < m_ncandidates; iPart++) {

      if (iPart >= m_maxCandidates) {
        B2WARNING("Maximum number of candidates exceeded in VariablesToEventBasedTree module. I will skip additional candidates");
        m_ncandidates = m_maxCandidates;
        break;
      }

      const Particle* particle = particlelist->getParticle(iPart);
      for (unsigned int iVar = 0; iVar < m_functions.size(); iVar++) {
        if (std::holds_alternative<double>(m_functions[iVar](particle))) {
          m_valuesDouble[iVar][iPart] = std::get<double>(m_functions[iVar](particle));
        } else if (std::holds_alternative<int>(m_functions[iVar](particle))) {
          m_valuesInt[iVar][iPart] = std::get<int>(m_functions[iVar](particle));
        } else if (std::holds_alternative<bool>(m_functions[iVar](particle))) {
          m_valuesInt[iVar][iPart] = std::get<bool>(m_functions[iVar](particle));
        }
      }
    }
    m_tree->get().Fill();
  }
}

void VariablesToEventBasedTreeModule::terminate()
{
  if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
    B2INFO("Writing TTree " << m_treeName);
    TDirectory::TContext directoryGuard(m_file.get());
    m_tree->write(m_file.get());

    const bool writeError = m_file->TestBit(TFile::kWriteError);
    m_file.reset();
    if (writeError) {
      B2FATAL("A write error occurred while saving '" << m_fileName  << "', please check if enough disk space is available.");
    }
  }
}
