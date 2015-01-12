/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtuplePDGCodeTool.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtuplePDGCodeTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();
  m_iQ = new int[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_PDG").c_str(), &m_iQ[iProduct], (strNames[iProduct] + "_PDG/I").c_str());
  }
}

void NtuplePDGCodeTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtuplePDGCodeTool::eval - no Particle found!");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_iQ[iProduct] = selparticles[iProduct]->getPDGCode();
  }
}
