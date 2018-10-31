/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2018 - Belle II Collaboration                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleiTOPCalibTool.h>
#include <analysis/variables/Variables.h>
#include <top/variables/TOPDigitVariables.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleiTOPCalibTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();

  m_nDigits = vector<float>(nDecayProducts);
  m_nReflected = vector<float>(nDecayProducts);
  m_maxGap = vector<float>(nDecayProducts);
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_nTOPDigits").c_str(), &m_nDigits[iProduct], (strNames[iProduct] + "_nTOPDigits/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_TOPDigitGap").c_str(), &m_maxGap[iProduct], (strNames[iProduct] + "_TOPDigitGap/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_nReflectedTOPDigits").c_str(), &m_nReflected[iProduct],
                   (strNames[iProduct] + "_nReflectedTOPDigits/F").c_str());
  }
}

void NtupleiTOPCalibTool::eval(const Particle* particle)
{
  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);

  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_nDigits[iProduct] = Variable::getTOPModuleDigitCount(selparticles[iProduct]);
    m_maxGap[iProduct] = Variable::getTOPModuleDigitGapSize(selparticles[iProduct]);
    m_nReflected[iProduct] = Variable::getNReflectedTOPModuleDigits(selparticles[iProduct]);
  }
}
