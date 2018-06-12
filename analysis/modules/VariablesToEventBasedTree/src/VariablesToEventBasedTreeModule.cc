/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Keck                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/modules/VariablesToEventBasedTree/VariablesToEventBasedTreeModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>
#include <framework/logging/Logger.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/utilities/MakeROOTCompatible.h>
#include <framework/core/ModuleParam.templateDetails.h>

#include <cmath>
#include <algorithm>

using namespace std;
using namespace Belle2;

// Register module in the framework
REG_MODULE(VariablesToEventBasedTree)


VariablesToEventBasedTreeModule::VariablesToEventBasedTreeModule() :
  Module(),
  m_tree("", DataStore::c_Persistent)
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

  addParam("fileName", m_fileName, "Name of ROOT file for output.", string("VariablesToEventBasedTree.root"));
  addParam("treeName", m_treeName, "Name of the NTuple in the saved file.", string("tree"));
  addParam("maxCandidates", m_maxCandidates, "The maximum number of candidates in the ParticleList per entry of the Tree.", 100u);

  std::tuple<std::string, std::map<int, unsigned int>> default_sampling{"", {}};
  addParam("sampling", m_sampling,
           "Tuple of variable name and a map of integer values and inverse sampling rate. E.g. (signal, {1: 0, 0:10}) selects all signal events and every 10th background event. Variable must be event-based.",
           default_sampling);

  m_file = nullptr;
}

void VariablesToEventBasedTreeModule::initialize()
{
  StoreObjPtr<ParticleList>().isRequired(m_particleList);

  // Initializing the output root file
  m_file = new TFile(m_fileName.c_str(), "RECREATE");
  if (!m_file->IsOpen()) {
    B2WARNING("Could not create file " << m_fileName);
    return;
  }

  m_file->cd();

  // check if TTree with that name already exists
  if (m_file->Get(m_treeName.c_str())) {
    B2WARNING("Tree with this name already exists: " << m_fileName);
    return;
  }

  m_variables = Variable::Manager::Instance().resolveCollections(m_variables);
  m_event_variables = Variable::Manager::Instance().resolveCollections(m_event_variables);

  m_tree.registerInDataStore(m_fileName + m_treeName, DataStore::c_DontWriteOut);
  m_tree.construct(m_treeName.c_str(), "");

  m_values.resize(m_variables.size());
  m_event_values.resize(m_event_variables.size());

  m_tree->get().Branch("__ncandidates__", &m_ncandidates, "__ncandidates__/I");
  m_tree->get().Branch("__weight__", &m_weight, "__weight__/F");

  for (unsigned int i = 0; i < m_event_variables.size(); ++i) {
    auto varStr = m_event_variables[i];
    m_tree->get().Branch(makeROOTCompatible(varStr).c_str(), &m_event_values[i], (makeROOTCompatible(varStr) + "/D").c_str());

    //also collection function pointers
    const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(varStr);
    if (!var) {
      B2ERROR("Variable '" << varStr << "' is not available in Variable::Manager!");
    } else {
      m_event_functions.push_back(var->function);
    }
  }

  for (unsigned int i = 0; i < m_variables.size(); ++i) {
    auto varStr = m_variables[i];
    m_values[i].resize(m_maxCandidates);
    m_tree->get().Branch(makeROOTCompatible(varStr).c_str(), &m_values[i][0],
                         (makeROOTCompatible(varStr) + "[__ncandidates__]/D").c_str());

    //also collection function pointers
    const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(varStr);
    if (!var) {
      B2ERROR("Variable '" << varStr << "' is not available in Variable::Manager!");
    } else {
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

  long target = std::lround(m_sampling_variable->function(nullptr));
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

  StoreObjPtr<ParticleList> particlelist(m_particleList);
  m_ncandidates = particlelist->getListSize();
  m_weight = getInverseSamplingRateWeight();
  if (m_weight > 0) {
    for (unsigned int iVar = 0; iVar < m_event_functions.size(); iVar++) {
      m_event_values[iVar] = m_event_functions[iVar](nullptr);
    }
    for (unsigned int iPart = 0; iPart < m_ncandidates; iPart++) {

      if (iPart >= m_maxCandidates) {
        B2WARNING("Maximum number of candidates exceeded in VariablesToEventBasedTree module. I will skip additional candidates");
        break;
      }

      const Particle* particle = particlelist->getParticle(iPart);
      for (unsigned int iVar = 0; iVar < m_functions.size(); iVar++) {
        m_values[iVar][iPart] = m_functions[iVar](particle);
      }
    }
    m_tree->get().Fill();
  }
}

void VariablesToEventBasedTreeModule::terminate()
{
  if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
    B2INFO("Writing TTree " << m_treeName);
    m_tree->write(m_file);

    const bool writeError = m_file->TestBit(TFile::kWriteError);
    if (writeError) {
      //m_file deleted first so we have a chance of closing it (though that will probably fail)
      delete m_file;
      B2FATAL("A write error occured while saving '" << m_fileName  << "', please check if enough disk space is available.");
    }

    B2INFO("Closing file " << m_fileName);
    delete m_file;
  }
}
