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
#include <framework/utilities/MakeROOTCompatible.h>

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
  std::vector<std::tuple<std::string, int, float, float, std::string, int, float, float>> emptylist_2d;
  addParam("particleList", m_particleList,
           "Name of particle list with reconstructed particles. If no list is provided the variables are saved once per event (only possible for event-type variables)",
           std::string(""));
  addParam("variables", m_variables,
           "List of variables to save. Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector.",
           emptylist);
  addParam("variables_2d", m_variables_2d,
           "List of variable pairs to save. Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector.",
           emptylist_2d);

  addParam("fileName", m_fileName, "Name of ROOT file for output.", string("VariablesToHistogram.root"));

  m_file = nullptr;
}

void VariablesToHistogramModule::initialize()
{
  if (not m_particleList.empty())
    StoreObjPtr<ParticleList>().isRequired(m_particleList);

  // Check if we can acces the given file
  m_file = new TFile(m_fileName.c_str(), "RECREATE");
  if (!m_file->IsOpen()) {
    B2WARNING("Could not create file " << m_fileName);
    return;
  }
  delete m_file;

  for (const auto& varTuple : m_variables) {
    std::string varStr;
    int varNbins = 0;
    float low = 0;
    float high = 0;
    std::tie(varStr, varNbins, low, high) = varTuple;
    std::string compatibleName = makeROOTCompatible(varStr);

    auto ptr = std::unique_ptr<StoreObjPtr<RootMergeable<TH1D>>>(new StoreObjPtr<RootMergeable<TH1D>>("", DataStore::c_Persistent));
    ptr->registerInDataStore(m_fileName + varStr, DataStore::c_DontWriteOut);
    ptr->construct(compatibleName.c_str(), compatibleName.c_str(), varNbins, low, high);
    // Create histogram in memory and do not associate them with a TFile
    (*ptr)->get().SetDirectory(0);
    m_hists.push_back(std::move(ptr));

    //also collection function pointers
    const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(varStr);
    if (!var) {
      B2ERROR("Variable '" << varStr << "' is not available in Variable::Manager!");
    } else {
      m_functions.push_back(var->function);
    }
  }

  for (const auto& varTuple : m_variables_2d) {
    std::string varStr1;
    int varNbins1 = 0;
    float low1 = 0;
    float high1 = 0;
    std::string varStr2;
    int varNbins2 = 0;
    float low2 = 0;
    float high2 = 0;
    std::tie(varStr1, varNbins1, low1, high1, varStr2, varNbins2, low2, high2) = varTuple;
    std::string compatibleName1 = makeROOTCompatible(varStr1);
    std::string compatibleName2 = makeROOTCompatible(varStr2);

    auto ptr2d = std::unique_ptr<StoreObjPtr<RootMergeable<TH2D>>>(new StoreObjPtr<RootMergeable<TH2D>>("", DataStore::c_Persistent));
    ptr2d->registerInDataStore(m_fileName + varStr1 + varStr2, DataStore::c_DontWriteOut);
    ptr2d->construct((compatibleName1 + compatibleName2).c_str(), (compatibleName1 + compatibleName2).c_str(),
                     varNbins1, low1, high1, varNbins2, low2, high2);
    (*ptr2d)->get().SetDirectory(0);
    m_2d_hists.push_back(std::move(ptr2d));

    //also collection function pointers
    const Variable::Manager::Var* var1 = Variable::Manager::Instance().getVariable(varStr1);
    if (!var1) {
      B2ERROR("Variable '" << varStr1 << "' is not available in Variable::Manager!");
    } else {
      m_functions_2d_1.push_back(var1->function);
    }

    //also collection function pointers
    const Variable::Manager::Var* var2 = Variable::Manager::Instance().getVariable(varStr2);
    if (!var2) {
      B2ERROR("Variable '" << varStr2 << "' is not available in Variable::Manager!");
    } else {
      m_functions_2d_2.push_back(var2->function);
    }
  }

}

void VariablesToHistogramModule::event()
{
  unsigned int nVars = m_variables.size();
  unsigned int nVars_2d = m_variables_2d.size();
  std::vector<float> vars(nVars);
  std::vector<float> vars_2d_1(nVars_2d);
  std::vector<float> vars_2d_2(nVars_2d);

  if (m_particleList.empty()) {
    for (unsigned int iVar = 0; iVar < nVars; iVar++) {
      vars[iVar] = m_functions[iVar](nullptr);
      (*m_hists[iVar])->get().Fill(vars[iVar]);
    }
    for (unsigned int iVar = 0; iVar < nVars_2d; iVar++) {
      vars_2d_1[iVar] = m_functions_2d_1[iVar](nullptr);
      vars_2d_2[iVar] = m_functions_2d_2[iVar](nullptr);
      (*m_2d_hists[iVar])->get().Fill(vars_2d_1[iVar], vars_2d_2[iVar]);
    }

  } else {
    StoreObjPtr<ParticleList> particlelist(m_particleList);
    unsigned int nPart = particlelist->getListSize();
    for (unsigned int iPart = 0; iPart < nPart; iPart++) {
      const Particle* particle = particlelist->getParticle(iPart);
      for (unsigned int iVar = 0; iVar < nVars; iVar++) {
        vars[iVar] = m_functions[iVar](particle);
        (*m_hists[iVar])->get().Fill(vars[iVar]);
      }
      for (unsigned int iVar = 0; iVar < nVars_2d; iVar++) {
        vars_2d_1[iVar] = m_functions_2d_1[iVar](particle);
        vars_2d_2[iVar] = m_functions_2d_2[iVar](particle);
        (*m_2d_hists[iVar])->get().Fill(vars_2d_1[iVar], vars_2d_2[iVar]);
      }
    }
  }
}

void VariablesToHistogramModule::terminate()
{
  if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
    B2INFO("Writing Histogram " << m_treeName);
    // Initializing the output root file
    m_file = new TFile(m_fileName.c_str(), "RECREATE");
    if (!m_file->IsOpen()) {
      B2WARNING("Could not create file " << m_fileName);
      return;
    }

    m_file->cd();
    unsigned int nVars = m_variables.size();
    for (unsigned int iVar = 0; iVar < nVars; iVar++) {
      (*m_hists[iVar])->write(m_file);
    }
    unsigned int nVars_2d = m_variables_2d.size();
    for (unsigned int iVar = 0; iVar < nVars_2d; iVar++) {
      (*m_2d_hists[iVar])->write(m_file);
    }

    B2INFO("Closing file " << m_fileName);
    delete m_file;
  }
}
