/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/VariablesToHistogram/VariablesToHistogramModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/VariableManager/Manager.h>
#include <framework/logging/Logger.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/utilities/MakeROOTCompatible.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/utilities/RootFileCreationManager.h>

#include <memory>

using namespace std;
using namespace Belle2;

// Register module in the framework
REG_MODULE(VariablesToHistogram)


VariablesToHistogramModule::VariablesToHistogramModule() :
  Module()
{
  //Set module properties
  setDescription("Calculate variables specified by the user for a given ParticleList and save them into one or two dimensional histograms.");
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
  addParam("directory", m_directory, "Directory for all histograms **inside** the file to allow for histograms from multiple "
           "particlelists in the same file without conflicts", m_directory);

  m_file = nullptr;
}

void VariablesToHistogramModule::initialize()
{
  if (not m_particleList.empty())
    StoreObjPtr<ParticleList>().isRequired(m_particleList);

  // Check if we can acces the given file
  m_file = RootFileCreationManager::getInstance().getFile(m_fileName);
  if (!m_file) return;
  // Make sure we don't disturb the global directory for other modules, friggin side effects everywhere
  TDirectory::TContext directoryGuard(m_file.get());
  if (not m_directory.empty()) {
    m_directory = makeROOTCompatible(m_directory);
    m_file->mkdir(m_directory.c_str());
    m_file->cd(m_directory.c_str());
  }

  for (const auto& varTuple : m_variables) {
    std::string varStr;
    int varNbins = 0;
    float low = 0;
    float high = 0;
    std::tie(varStr, varNbins, low, high) = varTuple;
    std::string compatibleName = makeROOTCompatible(varStr);

    auto ptr = std::make_unique<StoreObjPtr<RootMergeable<TH1D>>>("", DataStore::c_Persistent);
    ptr->registerInDataStore(m_fileName + m_directory + varStr, DataStore::c_DontWriteOut);
    ptr->construct(compatibleName.c_str(), compatibleName.c_str(), varNbins, low, high);
    m_hists.emplace_back(std::move(ptr));

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

    auto ptr2d = std::make_unique<StoreObjPtr<RootMergeable<TH2D>>>("", DataStore::c_Persistent);
    ptr2d->registerInDataStore(m_fileName + m_directory + varStr1 + varStr2, DataStore::c_DontWriteOut);
    ptr2d->construct((compatibleName1 + compatibleName2).c_str(), (compatibleName1 + compatibleName2).c_str(),
                     varNbins1, low1, high1, varNbins2, low2, high2);
    m_2d_hists.emplace_back(std::move(ptr2d));

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
      if (std::holds_alternative<double>(m_functions[iVar](nullptr))) {
        vars[iVar] = std::get<double>(m_functions[iVar](nullptr));
      } else if (std::holds_alternative<int>(m_functions[iVar](nullptr))) {
        vars[iVar] = std::get<int>(m_functions[iVar](nullptr));
      } else if (std::holds_alternative<bool>(m_functions[iVar](nullptr))) {
        vars[iVar] = std::get<bool>(m_functions[iVar](nullptr));
      }
      (*m_hists[iVar])->get().Fill(vars[iVar]);
    }
    for (unsigned int iVar = 0; iVar < nVars_2d; iVar++) {
      if (std::holds_alternative<double>(m_functions_2d_1[iVar](nullptr))) {
        vars_2d_1[iVar] = std::get<double>(m_functions_2d_1[iVar](nullptr));
      } else if (std::holds_alternative<int>(m_functions_2d_1[iVar](nullptr))) {
        vars_2d_1[iVar] = std::get<int>(m_functions_2d_1[iVar](nullptr));
      } else if (std::holds_alternative<bool>(m_functions_2d_1[iVar](nullptr))) {
        vars_2d_1[iVar] = std::get<bool>(m_functions_2d_1[iVar](nullptr));
      }
      if (std::holds_alternative<double>(m_functions_2d_2[iVar](nullptr))) {
        vars_2d_2[iVar] = std::get<double>(m_functions_2d_2[iVar](nullptr));
      } else if (std::holds_alternative<int>(m_functions_2d_2[iVar](nullptr))) {
        vars_2d_2[iVar] = std::get<int>(m_functions_2d_2[iVar](nullptr));
      } else if (std::holds_alternative<bool>(m_functions_2d_2[iVar](nullptr))) {
        vars_2d_2[iVar] = std::get<bool>(m_functions_2d_2[iVar](nullptr));
      }
      (*m_2d_hists[iVar])->get().Fill(vars_2d_1[iVar], vars_2d_2[iVar]);
    }

  } else {
    StoreObjPtr<ParticleList> particlelist(m_particleList);
    unsigned int nPart = particlelist->getListSize();
    for (unsigned int iPart = 0; iPart < nPart; iPart++) {
      const Particle* particle = particlelist->getParticle(iPart);
      for (unsigned int iVar = 0; iVar < nVars; iVar++) {
        if (std::holds_alternative<double>(m_functions[iVar](particle))) {
          vars[iVar] = std::get<double>(m_functions[iVar](particle));
        } else if (std::holds_alternative<int>(m_functions[iVar](particle))) {
          vars[iVar] = std::get<int>(m_functions[iVar](particle));
        } else if (std::holds_alternative<bool>(m_functions[iVar](particle))) {
          vars[iVar] = std::get<bool>(m_functions[iVar](particle));
        }
        (*m_hists[iVar])->get().Fill(vars[iVar]);
      }
      for (unsigned int iVar = 0; iVar < nVars_2d; iVar++) {
        if (std::holds_alternative<double>(m_functions_2d_1[iVar](particle))) {
          vars_2d_1[iVar] = std::get<double>(m_functions_2d_1[iVar](particle));
        } else if (std::holds_alternative<int>(m_functions_2d_1[iVar](particle))) {
          vars_2d_1[iVar] = std::get<int>(m_functions_2d_1[iVar](particle));
        } else if (std::holds_alternative<bool>(m_functions_2d_1[iVar](particle))) {
          vars_2d_1[iVar] = std::get<bool>(m_functions_2d_1[iVar](particle));
        }
        if (std::holds_alternative<double>(m_functions_2d_2[iVar](particle))) {
          vars_2d_2[iVar] = std::get<double>(m_functions_2d_2[iVar](particle));
        } else if (std::holds_alternative<int>(m_functions_2d_2[iVar](particle))) {
          vars_2d_2[iVar] = std::get<int>(m_functions_2d_2[iVar](particle));
        } else if (std::holds_alternative<bool>(m_functions_2d_2[iVar](particle))) {
          vars_2d_2[iVar] = std::get<bool>(m_functions_2d_2[iVar](particle));
        }
        (*m_2d_hists[iVar])->get().Fill(vars_2d_1[iVar], vars_2d_2[iVar]);
      }
    }
  }
}

void VariablesToHistogramModule::terminate()
{
  if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
    TDirectory::TContext directoryGuard(m_file.get());
    if (not m_directory.empty()) {
      m_file->cd(m_directory.c_str());
    }
    B2INFO("Writing Histograms to " << gDirectory->GetPath());
    unsigned int nVars = m_variables.size();
    for (unsigned int iVar = 0; iVar < nVars; iVar++) {
      (*m_hists[iVar])->write(gDirectory);
    }
    unsigned int nVars_2d = m_variables_2d.size();
    for (unsigned int iVar = 0; iVar < nVars_2d; iVar++) {
      (*m_2d_hists[iVar])->write(gDirectory);
    }

    const bool writeError = m_file->TestBit(TFile::kWriteError);
    m_file.reset();
    if (writeError) {
      B2FATAL("A write error occured while saving '" << m_fileName  << "', please check if enough disk space is available.");
    }
  }
}
