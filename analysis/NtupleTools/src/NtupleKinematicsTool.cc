/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleKinematicsTool.h>
#include <TBranch.h>
void NtupleKinematicsTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();
  m_fP = new float[nDecayProducts];
  m_fPx = new float[nDecayProducts];
  m_fPy = new float[nDecayProducts];
  m_fPz = new float[nDecayProducts];
  m_fE = new float[nDecayProducts];
  m_fM = new float[nDecayProducts];
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_P").c_str(), &m_fP[iProduct], (strNames[iProduct] + "_P/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Px").c_str(), &m_fPx[iProduct], (strNames[iProduct] + "_Px/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Py").c_str(), &m_fPy[iProduct], (strNames[iProduct] + "_Py/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Pz").c_str(), &m_fPz[iProduct], (strNames[iProduct] + "_Pz/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_E").c_str(), &m_fE[iProduct], (strNames[iProduct] + "_E/F").c_str());
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
    m_fP[iProduct] = selparticles[iProduct]->getP();
    m_fPx[iProduct] = selparticles[iProduct]->getPx();
    m_fPy[iProduct] = selparticles[iProduct]->getPy();
    m_fPz[iProduct] = selparticles[iProduct]->getPz();
    m_fE[iProduct] = selparticles[iProduct]->getEnergy();
    m_fM[iProduct] = selparticles[iProduct]->getMass();
  }
}
