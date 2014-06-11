/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleCustomFloatsTool.h>
#include <framework/logging/Logger.h>
#include <TBranch.h>
#include <boost/algorithm/string.hpp>

using namespace Belle2;
using namespace std;

void NtupleCustomFloatsTool::setupTree()
{

  // Initialise function pointers of the variables
  if (m_strOption.empty()) {
    B2ERROR("No variables specified for the NtupleCustomFloatsTool!");
  }
  B2INFO("Option is: " << m_strOption);
  boost::split(m_strVarNames, m_strOption, boost::is_any_of(":"));

  int nVars = m_strVarNames.size();
  for (int iVar = 0; iVar < nVars; iVar++) {
    const VariableManager::Var* var = VariableManager::Instance().getVariable(m_strVarNames[iVar]);
    if (!var) {
      B2ERROR("Variable '" << m_strVarNames[iVar] << "' is not available in VariableManager!");
    } else {
      m_functions.push_back(var->function);
    }
  }

  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();

  m_fVars = new float[nDecayProducts * nVars];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    for (int iVar = 0; iVar < nVars; iVar++) {
      int iPos = iProduct * nDecayProducts + iVar;
      m_tree->Branch((strNames[iProduct] + "__" + m_strVarNames[iVar]).c_str(), &m_fVars[iPos], (strNames[iProduct] + "__" + m_strVarNames[iVar] + "/F").c_str());
    }
  }
}

void NtupleCustomFloatsTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleCustomFloatsTool::eval - ERROR, no Particle found!\n");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  int nVars = m_strVarNames.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    for (int iVar = 0; iVar < nVars; iVar++) {
      int iPos = iProduct * nDecayProducts + iVar;
      m_fVars[iPos] = m_functions[iVar](selparticles[iProduct]);
    }
  }
}
