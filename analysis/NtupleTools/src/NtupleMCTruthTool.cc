/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo, Anze Zupanc           *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleMCTruthTool.h>
#include <analysis/utility/MCMatching.h>
#include <mdst/dataobjects/MCParticle.h>

#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleMCTruthTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();
  m_iTruthID = new int[nDecayProducts];
  m_iTruthIDMatch = new int[nDecayProducts];
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_mcPDG").c_str(), &m_iTruthID[iProduct], (strNames[iProduct] + "_mcPDG/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_mcErrors").c_str(), &m_iTruthIDMatch[iProduct],
                   (strNames[iProduct] + "_mcErrors/I").c_str());
  }
}

void NtupleMCTruthTool::deallocateMemory()
{
  delete [] m_iTruthID;
  delete [] m_iTruthIDMatch;
}

void NtupleMCTruthTool::eval(const Particle* particle)
{
  StoreArray<MCParticle> mcParticles;

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_iTruthID[iProduct] = 0;

    const MCParticle* mcparticle = selparticles[iProduct]->getRelatedTo<MCParticle>();
    if (mcparticle) {
      m_iTruthID[iProduct]      = mcparticle->getPDG();
    }
    m_iTruthIDMatch[iProduct] = MCMatching::getMCErrors(selparticles[iProduct], mcparticle);
  }
}
