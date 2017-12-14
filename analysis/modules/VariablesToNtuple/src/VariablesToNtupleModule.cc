/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Pulvermacher                                   *
*               Thomas Keck                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/modules/VariablesToNtuple/VariablesToNtupleModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>
#include <framework/logging/Logger.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/utilities/MakeROOTCompatible.h>

#include <cmath>
#include <algorithm>

using namespace std;
using namespace Belle2;

// Register module in the framework
REG_MODULE(VariablesToNtuple)


VariablesToNtupleModule::VariablesToNtupleModule() :
  Module(),
  m_tree("", DataStore::c_Persistent)
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

  m_file = nullptr;
}

void VariablesToNtupleModule::initialize()
{
  if (not m_particleList.empty())
    StoreObjPtr<ParticleList>().isRequired(m_particleList);


  // Initializing the output root file
  if (m_fileName.empty()) {
    B2FATAL("Output root file name is not set. Please set a vaild root output file name (\"fileName\" module parameter).");
  }

  m_file = new TFile(m_fileName.c_str(), "RECREATE");
  if (!m_file->IsOpen()) {
    B2ERROR("Could not create file \"" << m_fileName <<
            "\". Please set a vaild root output file name (\"fileName\" module parameter).");
    return;
  }

  m_file->cd();

  // check if TTree with that name already exists
  if (m_file->Get(m_treeName.c_str())) {
    B2WARNING("Tree with this name already exists: \"" << m_fileName << "\"");
    return;
  }

  m_variables = Variable::Manager::Instance().resolveCollections(m_variables);

  // root wants var1:var2:...
  string varlist = "__weight__";
  for (const string& varStr : m_variables) {
    varlist += ":";
    varlist += makeROOTCompatible(varStr);

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

  m_tree.registerInDataStore(m_fileName + m_treeName, DataStore::c_DontWriteOut);
  m_tree.construct(m_treeName.c_str(), "", varlist.c_str());
  m_tree->get().SetBasketSize("*", 1600);
  m_tree->get().SetCacheSize(100000);
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
  unsigned int nVars = m_variables.size();
  std::vector<float> vars(nVars + 1);

  if (m_particleList.empty()) {
    vars[0] = getInverseSamplingRateWeight(nullptr);
    if (vars[0] > 0) {
      for (unsigned int iVar = 0; iVar < nVars; iVar++) {
        vars[iVar + 1] = m_functions[iVar](nullptr);
      }
      m_tree->get().Fill(vars.data());
    }

  } else {
    StoreObjPtr<ParticleList> particlelist(m_particleList);
    unsigned int nPart = particlelist->getListSize();
    for (unsigned int iPart = 0; iPart < nPart; iPart++) {
      const Particle* particle = particlelist->getParticle(iPart);
      vars[0] = getInverseSamplingRateWeight(particle);
      if (vars[0] > 0) {
        for (unsigned int iVar = 0; iVar < nVars; iVar++) {
          vars[iVar + 1] = m_functions[iVar](particle);
        }
        m_tree->get().Fill(vars.data());
      }
    }
  }
}

void VariablesToNtupleModule::terminate()
{
  if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
    B2INFO("Writing NTuple " << m_treeName);
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
