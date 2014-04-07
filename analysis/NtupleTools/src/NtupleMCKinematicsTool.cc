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
  int nDecayProducts = strNames.size();
  m_fTruthP = new float[nDecayProducts];
  m_fTruthP4 = new float*[nDecayProducts];
  m_fTruthM = new float[nDecayProducts];
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_TruthP").c_str(), &m_fTruthP[iProduct], (strNames[iProduct] + "_TruthP/F").c_str());
    m_fTruthP4[iProduct] = new float[4];
    m_tree->Branch((strNames[iProduct] + "_TruthP4").c_str(), &m_fTruthP4[iProduct][0], (strNames[iProduct] + "_TruthP4[4]/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_TruthM").c_str(), &m_fTruthM[iProduct], (strNames[iProduct] + "_TruthM/F").c_str());
  }
}

void NtupleMCKinematicsTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleMCKinematicsTool::eval - ERROR, no Particle found!\n");
    return;
  }
  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    const MCParticle* mcparticle = DataStore::getRelated<MCParticle>(selparticles[iProduct]);

    if (mcparticle) {
      const TLorentzVector mcparticle_4vec = mcparticle->get4Vector();
      m_fTruthP[iProduct]  = mcparticle_4vec.P();
      m_fTruthP4[iProduct][0] = mcparticle_4vec.Px();
      m_fTruthP4[iProduct][1] = mcparticle_4vec.Py();
      m_fTruthP4[iProduct][2] = mcparticle_4vec.Pz();
      m_fTruthP4[iProduct][3]  = mcparticle->getEnergy();
      m_fTruthM[iProduct]  = mcparticle->getMass();
    }
  }
}
