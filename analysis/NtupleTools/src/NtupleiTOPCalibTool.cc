/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
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

  m_nDigits = new float[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_nTOPDigits").c_str(), &m_nDigits[iProduct], (strNames[iProduct] + "_nDigits/F").c_str());
  }
}

void NtupleiTOPCalibTool::eval(const Particle* particle)
{
  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);

  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_nDigits[iProduct] = Variable::getTOPModuleDigitCount(selparticles[iProduct]);
  }
}
