/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleMomentumUncertaintyTool.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleMomentumUncertaintyTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();
  m_fErrP4 = new float*[nDecayProducts];
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_fErrP4[iProduct] = new float[4];
    m_tree->Branch((strNames[iProduct] + "_ErrP4").c_str(), &m_fErrP4[iProduct][0], (strNames[iProduct] + "_ErrP4[4]/F").c_str());
  }
}

void NtupleMomentumUncertaintyTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleMomentumUncertaintyTool::eval - ERROR, no Particle found!\n");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_fErrP4[iProduct][0] = TMath::Sqrt(selparticles[iProduct]->getMomentumErrorMatrix()[0][0]);
    m_fErrP4[iProduct][1] = TMath::Sqrt(selparticles[iProduct]->getMomentumErrorMatrix()[1][1]);
    m_fErrP4[iProduct][2] = TMath::Sqrt(selparticles[iProduct]->getMomentumErrorMatrix()[2][2]);
    m_fErrP4[iProduct][3] = TMath::Sqrt(selparticles[iProduct]->getMomentumErrorMatrix()[3][3]);
  }
}
