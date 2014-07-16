/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Luigi Li Gioi                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleKinematicsTool.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleKinematicsTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();
  m_iQ = new Char_t[nDecayProducts];
  m_fP = new float[nDecayProducts];
  m_fM = new float[nDecayProducts];
  m_fP4 = new float*[nDecayProducts];
  m_fEP4 = new float*[nDecayProducts];
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_Q").c_str(), &m_iQ[iProduct], (strNames[iProduct] + "_Q/B").c_str());
    m_tree->Branch((strNames[iProduct] + "_P").c_str(), &m_fP[iProduct], (strNames[iProduct] + "_P/F").c_str());
    m_fP4[iProduct] = new float[4];
    m_tree->Branch((strNames[iProduct] + "_P4").c_str(), &m_fP4[iProduct][0], (strNames[iProduct] + "_P4[4]/F").c_str());
    m_fEP4[iProduct] = new float[4];
    m_tree->Branch((strNames[iProduct] + "_ErrP4").c_str(), &m_fEP4[iProduct][0], (strNames[iProduct] + "_ErrP4[4]/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_M").c_str(), &m_fM[iProduct], (strNames[iProduct] + "_M/F").c_str());
  }
}

void NtupleKinematicsTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleKinematicsTool::eval - ERROR, no Particle found!\n");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_iQ[iProduct] = selparticles[iProduct]->getCharge();
    m_fP[iProduct] = selparticles[iProduct]->getP();
    m_fP4[iProduct][0] = selparticles[iProduct]->getPx();
    m_fP4[iProduct][1] = selparticles[iProduct]->getPy();
    m_fP4[iProduct][2] = selparticles[iProduct]->getPz();
    m_fP4[iProduct][3] = selparticles[iProduct]->getEnergy();
    m_fEP4[iProduct][0] = TMath::Sqrt(selparticles[iProduct]->getMomentumVertexErrorMatrix()[0][0]);
    m_fEP4[iProduct][1] = TMath::Sqrt(selparticles[iProduct]->getMomentumVertexErrorMatrix()[1][1]);
    m_fEP4[iProduct][2] = TMath::Sqrt(selparticles[iProduct]->getMomentumVertexErrorMatrix()[2][2]);
    m_fEP4[iProduct][3] = TMath::Sqrt(selparticles[iProduct]->getMomentumVertexErrorMatrix()[3][3]);
    m_fM[iProduct] = selparticles[iProduct]->getMass();
  }
}
