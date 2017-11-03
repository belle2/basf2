/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleMCKinematicsTool.h>
#include <mdst/dataobjects/MCParticle.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleMCKinematicsTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  m_nDecayProducts = strNames.size();
  m_fTruthP = new float[m_nDecayProducts];
  m_fTruthP4 = new float*[m_nDecayProducts];
  m_fTruthM = new float[m_nDecayProducts];
  for (int iProduct = 0; iProduct < m_nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_TruthP").c_str(), &m_fTruthP[iProduct], (strNames[iProduct] + "_TruthP/F").c_str());
    m_fTruthP4[iProduct] = new float[4];
    m_tree->Branch((strNames[iProduct] + "_TruthP4").c_str(), &m_fTruthP4[iProduct][0], (strNames[iProduct] + "_TruthP4[4]/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_TruthM").c_str(), &m_fTruthM[iProduct], (strNames[iProduct] + "_TruthM/F").c_str());
  }
}

void NtupleMCKinematicsTool::deallocateMemory()
{
  for (int iProduct = 0; iProduct < m_nDecayProducts; iProduct++) {
    delete [] m_fTruthP4[iProduct];
  }
  delete [] m_fTruthP4;
  delete [] m_fTruthP;
  delete [] m_fTruthM;
}

void NtupleMCKinematicsTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleMCKinematicsTool::eval - no Particle found!");
    return;
  }
  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    const MCParticle* mcparticle = selparticles[iProduct]->getRelatedTo<MCParticle>();

    if (mcparticle) {
      const TLorentzVector mcparticle_4vec = mcparticle->get4Vector();
      m_fTruthP[iProduct]  = mcparticle_4vec.P();
      m_fTruthP4[iProduct][0] = mcparticle_4vec.Px();
      m_fTruthP4[iProduct][1] = mcparticle_4vec.Py();
      m_fTruthP4[iProduct][2] = mcparticle_4vec.Pz();
      m_fTruthP4[iProduct][3]  = mcparticle->getEnergy();
      m_fTruthM[iProduct]  = mcparticle->getMass();
    } else {
      m_fTruthP[iProduct]     = 0.0;
      m_fTruthP4[iProduct][0] = 0.0;
      m_fTruthP4[iProduct][1] = 0.0;
      m_fTruthP4[iProduct][2] = 0.0;
      m_fTruthP4[iProduct][3] = 0.0;
      m_fTruthM[iProduct]     = 0.0;
    }
  }
}
