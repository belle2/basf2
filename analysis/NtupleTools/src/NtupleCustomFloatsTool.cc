/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Anze Zupanc                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleCustomFloatsTool.h>
#include <framework/logging/Logger.h>
#include <TBranch.h>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

using namespace Belle2;
using namespace std;

void NtupleCustomFloatsTool::setupTree()
{

  // Initialise function pointers of the variables
  if (m_strOption.empty()) {
    B2ERROR("No variables specified for the NtupleCustomFloatsTool!");
  }
  //B2INFO("Option is: " << m_strOption);
  boost::split(m_strVarNames, m_strOption, boost::is_any_of(":"));

  int nVars = m_strVarNames.size();
  for (int iVar = 0; iVar < nVars; iVar++) {
    const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(m_strVarNames[iVar]);
    if (!var) {
      B2ERROR("Variable '" << m_strVarNames[iVar] << "' is not available in Variable::Manager!");
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
      string varName = m_strVarNames[iVar];
      varName.erase(std::remove(varName.begin(), varName.end(), '('), varName.end());
      varName.erase(std::remove(varName.begin(), varName.end(), ')'), varName.end());
      varName.erase(std::remove(varName.begin(), varName.end(), ','), varName.end());

      // getExtraInfoVariableName -> VariableName
      boost::erase_all(varName, "getExtraInfo");

      // daughter0VariableName -> d0_VariableName
      boost::regex re("daughter([0-9])");
      varName = boost::regex_replace(varName, re, "d$1_");

      // daughterInvariantMass01 -> d01_M
      re = boost::regex("daughterInvariantMass(\\d+)");
      varName = boost::regex_replace(varName, re, "d$1_M");

      // decayAngle0 -> d0_decayAngle
      re = boost::regex("decayAngle([0-9])");
      varName = boost::regex_replace(varName, re, "d$1_decayAngle");

      // daughterAngle01 -> d01_angle
      re = boost::regex("daughterAngle(\\d+)");
      varName = boost::regex_replace(varName, re, "d$1_angle");

      // massDifference0 -> DeltaM0
      re = boost::regex("massDifference([0-9])");
      varName = boost::regex_replace(varName, re, "DeltaM$1");

      // massDifferenceError0 -> ErrDeltaM0
      re = boost::regex("massDifferenceError([0-9])");
      varName = boost::regex_replace(varName, re, "ErrDeltaM$1");

      // massDifferenceSignificance0 -> SigDeltaM0
      re = boost::regex("massDifferenceSignificance([0-9])");
      varName = boost::regex_replace(varName, re, "SigDeltaM$1");

      varName = (strNames[iProduct] + "__" + varName);
      m_tree->Branch(varName.c_str(), &m_fVars[iPos], (varName + "/F").c_str());

      B2INFO("   " << iPos << ") " << m_strVarNames[iVar] << " -> " << varName);
    }
  }
}

void NtupleCustomFloatsTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleCustomFloatsTool::eval - no Particle found!");
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
