/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleExtraEnergyTool.h>

#include <analysis/utility/PSelectorFunctions.h>

#include <TBranch.h>

using namespace std;
using namespace Belle2;

void NtupleExtraEnergyTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();

  m_extraE = new float[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_Eextra").c_str(),   &m_extraE[iProduct], (strNames[iProduct] + "_Eextra/F").c_str());
  }
}

void NtupleExtraEnergyTool::eval(const Particle* particle)
{
  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);

  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_extraE[iProduct]   = analysis::extraEnergy(selparticles[iProduct]);
  }
}
