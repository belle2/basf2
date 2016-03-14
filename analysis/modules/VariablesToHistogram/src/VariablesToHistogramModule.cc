/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Keck                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/modules/VariablesToHistogram/VariablesToHistogramModule.h>

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
REG_MODULE(VariablesToHistogram)


VariablesToHistogramModule::VariablesToHistogramModule() :
  Module()
{
  //Set module properties
  setDescription("Calculate variables specified by the user for a given ParticleList and save them into a TH1F.");
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

  std::vector<std::tuple<std::string, int, float, float>> emptylist;
  addParam("particleList", m_particleList,
           "Name of particle list with reconstructed particles. If no list is provided the variables are saved once per event (only possible for event-type variables)",
           std::string(""));
  addParam("variables", m_variables,
           "List of variables to save. Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector.",
           emptylist);

  addParam("fileName", m_fileName, "Name of ROOT file for output.", string("VariablesToHistogram.root"));

  m_file = nullptr;
}

void VariablesToHistogramModule::initialize()
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

  for (const auto& varTuple : m_variables) {
    std::string varStr;
    int varNbins = 0;
    float low = 0;
    float high = 0;
    std::tie(varStr, varNbins, low, high) = varTuple;
    std::string compatibleName = Variable::makeROOTCompatible(varStr);
    auto ptr = std::unique_ptr<StoreObjPtr<RootMergeable<TH1F>>>(new StoreObjPtr<RootMergeable<TH1F>>("", DataStore::c_Persistent));
    ptr->registerInDataStore(m_fileName + varStr, DataStore::c_DontWriteOut);
    ptr->construct(compatibleName.c_str(), compatibleName.c_str(), varNbins, low, high);
    m_hists.push_back(std::move(ptr));

    //also collection function pointers
    const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(varStr);
    if (!var) {
      B2ERROR("Variable '" << varStr << "' is not available in Variable::Manager!");
    } else {
      m_functions.push_back(var->function);
    }
  }

}

void VariablesToHistogramModule::event()
{
  unsigned int nVars = m_variables.size();
  std::vector<float> vars(nVars);

  if (m_particleList.empty()) {
    for (unsigned int iVar = 0; iVar < nVars; iVar++) {
      (*m_hists[iVar])->get().Fill(m_functions[iVar](nullptr));
    }

  } else {
    StoreObjPtr<ParticleList> particlelist(m_particleList);
    unsigned int nPart = particlelist->getListSize();
    for (unsigned int iPart = 0; iPart < nPart; iPart++) {
      const Particle* particle = particlelist->getParticle(iPart);
      for (unsigned int iVar = 0; iVar < nVars; iVar++) {
        vars[iVar] = m_functions[iVar](particle);
        (*m_hists[iVar])->get().Fill(m_functions[iVar](particle));
      }
    }
  }
}

void VariablesToHistogramModule::terminate()
{
  if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
    B2INFO("Writing Histogram " << m_treeName);
    unsigned int nVars = m_variables.size();
    for (unsigned int iVar = 0; iVar < nVars; iVar++) {
      (*m_hists[iVar])->write(m_file);
    }
    B2INFO("Closing file " << m_fileName);
    delete m_file;
  }
}
