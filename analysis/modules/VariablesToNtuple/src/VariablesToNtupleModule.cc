/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Pulvermacher                                   *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/modules/VariablesToNtuple/VariablesToNtupleModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>
#include <framework/logging/Logger.h>
#include <framework/pcore/ProcHandler.h>

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
  setDescription("Calculate variables specified by the user for a given ParticleList and save them into a TNtuple.");
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

  vector<string> emptylist;
  addParam("particleList", m_particleList, "Name of particle list with reconstructed particles. If no list is provided the variables are saved once per event (only possible for event-type variables)", std::string(""));
  addParam("variables", m_variables, "List of variables to save. Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector.", emptylist);

  addParam("fileName", m_fileName, "Name of ROOT file for output.", string("VariablesToNtuple.root"));
  addParam("treeName", m_treeName, "Name of the NTuple in the saved file.", string("ntuple"));

  m_file = nullptr;
}

void VariablesToNtupleModule::initialize()
{
  if (not m_particleList.empty())
    StoreObjPtr<ParticleList>::required(m_particleList);

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

  // root wants var1:var2:...
  string varlist;
  bool first = true;
  for (const string & varStr : m_variables) {
    if (!first)
      varlist += ":";
    varlist += Variable::makeROOTCompatible(varStr);
    first = false;

    //also collection function pointers
    const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(varStr);
    if (!var) {
      B2ERROR("Variable '" << varStr << "' is not available in Variable::Manager!");
    } else {
      m_functions.push_back(var->function);
    }
  }

  m_tree.registerInDataStore(m_fileName + m_treeName, DataStore::c_DontWriteOut);
  m_tree.construct(m_treeName.c_str(), "", varlist.c_str());
}

void VariablesToNtupleModule::event()
{
  unsigned int nVars = m_variables.size();
  std::vector<float> vars(nVars);

  if (m_particleList.empty()) {
    for (unsigned int iVar = 0; iVar < nVars; iVar++) {
      vars[iVar] = m_functions[iVar](nullptr);
    }
    m_tree->get().Fill(vars.data());

  } else {
    StoreObjPtr<ParticleList> particlelist(m_particleList);
    unsigned int nPart = particlelist->getListSize();
    for (unsigned int iPart = 0; iPart < nPart; iPart++) {
      const Particle* particle = particlelist->getParticle(iPart);
      for (unsigned int iVar = 0; iVar < nVars; iVar++) {
        vars[iVar] = m_functions[iVar](particle);
      }

      m_tree->get().Fill(vars.data());
    }
  }
}

void VariablesToNtupleModule::terminate()
{
  if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
    B2INFO("Writing NTuple " << m_treeName);
    m_file->cd();
    m_tree->get().Write();
    m_tree->get().SetDirectory(nullptr);

    B2INFO("Closing file " << m_fileName);
    delete m_file;
  }
}
