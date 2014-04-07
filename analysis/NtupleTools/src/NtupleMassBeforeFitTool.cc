/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleMassBeforeFitTool.h>
#include <analysis/utility/PSelectorFunctions.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleMassBeforeFitTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();

  m_invM     = new float[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_Mbf").c_str(),   &m_invM[iProduct], (strNames[iProduct] + "_Mbf/F").c_str());
  }
}

void NtupleMassBeforeFitTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleMassBeforeFitTool::eval - ERROR, no Particle found!\n");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_invM[iProduct]     = analysis::particleInvariantMass(selparticles[iProduct]);
  }
}
