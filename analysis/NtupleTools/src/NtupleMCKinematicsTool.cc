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
#include <generators/dataobjects/MCParticle.h>
#include <TBranch.h>

void NtupleMCKinematicsTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();
  m_fP = new float[nDecayProducts];
  m_fPx = new float[nDecayProducts];
  m_fPy = new float[nDecayProducts];
  m_fPz = new float[nDecayProducts];
  m_fE = new float[nDecayProducts];
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_TruthP").c_str(), &m_fP[iProduct], (strNames[iProduct] + "_TruthP/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_TruthPx").c_str(), &m_fPx[iProduct], (strNames[iProduct] + "_TruthPx/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_TruthPy").c_str(), &m_fPy[iProduct], (strNames[iProduct] + "_TruthPy/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_TruthPz").c_str(), &m_fPz[iProduct], (strNames[iProduct] + "_TruthPz/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_TruthE").c_str(), &m_fE[iProduct], (strNames[iProduct] + "_TruthE/F").c_str());
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

    if (!mcparticle) {
      printf("NtupleMCKinematicsTool::eval - WARNING no truth match found for this reco particle!\n");
    } else {
      const TLorentzVector mcparticle_4vec = mcparticle->get4Vector();
      m_fP[iProduct]  = mcparticle_4vec.P();
      m_fPx[iProduct] = mcparticle_4vec.Px();
      m_fPy[iProduct] = mcparticle_4vec.Py();
      m_fPz[iProduct] = mcparticle_4vec.Pz();
      m_fE[iProduct]  = mcparticle_4vec.Energy();

    }
  }
}
