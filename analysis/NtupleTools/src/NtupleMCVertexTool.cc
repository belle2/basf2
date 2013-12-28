/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Luigi Li Gioi                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleMCVertexTool.h>
#include <mdst/dataobjects/MCParticle.h>
#include <TBranch.h>

void NtupleMCVertexTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();
  m_fTruthX = new float[nDecayProducts];
  m_fTruthY = new float[nDecayProducts];
  m_fTruthZ = new float[nDecayProducts];
  m_fTruthRho = new float[nDecayProducts];
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_TruthX").c_str(), &m_fTruthX[iProduct], (strNames[iProduct] + "_TruthX/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_TruthY").c_str(), &m_fTruthY[iProduct], (strNames[iProduct] + "_TruthY/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_TruthZ").c_str(), &m_fTruthZ[iProduct], (strNames[iProduct] + "_TruthZ/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_TruthRho").c_str(), &m_fTruthRho[iProduct], (strNames[iProduct] + "_TruthRho/F").c_str());
  }
}

void NtupleMCVertexTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleMCVertexTool::eval - ERROR, no Particle found!\n");
    return;
  }
  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    const MCParticle* mcparticle = DataStore::getRelated<MCParticle>(selparticles[iProduct]);

    if (!mcparticle) {
      printf("NtupleMCVertexTool::eval - WARNING no truth match found for this reco particle!\n");
    } else {
      const TVector3 mcparticle_vert = mcparticle->getDecayVertex();
      m_fTruthX[iProduct] = mcparticle_vert.X();
      m_fTruthY[iProduct] = mcparticle_vert.Y();
      m_fTruthZ[iProduct] = mcparticle_vert.Z();
      m_fTruthRho[iProduct] = TMath::Sqrt(mcparticle_vert.Perp2());
    }
  }
}
