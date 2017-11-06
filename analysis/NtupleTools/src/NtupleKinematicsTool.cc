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
  m_nDecayProducts = strNames.size();
  m_fP = new float[m_nDecayProducts];
  m_fP4 = new float*[m_nDecayProducts];
  for (int iProduct = 0; iProduct < m_nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_P").c_str(), &m_fP[iProduct], (strNames[iProduct] + "_P/F").c_str());
    m_fP4[iProduct] = new float[4];
    m_tree->Branch((strNames[iProduct] + "_P4").c_str(), &m_fP4[iProduct][0], (strNames[iProduct] + "_P4[4]/F").c_str());
  }
}

void NtupleKinematicsTool::deallocateMemory()
{
  for (int iProduct = 0; iProduct < m_nDecayProducts; iProduct++) {
    delete [] m_fP4[iProduct];
  }
  delete [] m_fP4;
  delete [] m_fP;
}

void NtupleKinematicsTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleKinematicsTool::eval - no Particle found!");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_fP[iProduct] = selparticles[iProduct]->getP();
    m_fP4[iProduct][0] = selparticles[iProduct]->getPx();
    m_fP4[iProduct][1] = selparticles[iProduct]->getPy();
    m_fP4[iProduct][2] = selparticles[iProduct]->getPz();
    m_fP4[iProduct][3] = selparticles[iProduct]->getEnergy();
  }
}
