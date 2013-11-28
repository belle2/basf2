/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleMCReconstructibleTool.h>
#include <generators/dataobjects/MCParticle.h>
#include <TBranch.h>

void NtupleMCReconstructibleTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();
  m_iReconstructible = new int[nDecayProducts];
  m_iSeenInPXD = new int[nDecayProducts];
  m_iSeenInSVD = new int[nDecayProducts];
  m_iSeenInCDC = new int[nDecayProducts];
  m_iSeenInTOP = new int[nDecayProducts];
  m_iLastSeenInECL = new int[nDecayProducts];
  m_iLastSeenInKLM = new int[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_Reconstructible").c_str(), &m_iReconstructible[iProduct], (strNames[iProduct] + "_Reconstructible/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_SeenInPXD").c_str(), &m_iSeenInPXD[iProduct], (strNames[iProduct] + "_SeenInPXD/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_SeenInSVD").c_str(), &m_iSeenInSVD[iProduct], (strNames[iProduct] + "_SeenInSVD/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_SeenInCDC").c_str(), &m_iSeenInCDC[iProduct], (strNames[iProduct] + "_SeenInCDC/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_SeenInTOP").c_str(), &m_iSeenInTOP[iProduct], (strNames[iProduct] + "_SeenInTOP/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_LastSeenInECL").c_str(), &m_iLastSeenInECL[iProduct], (strNames[iProduct] + "_LastSeenInECL/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_LastSeenInKLM").c_str(), &m_iLastSeenInKLM[iProduct], (strNames[iProduct] + "_LastSeenInKLM/I").c_str());
  }
}

void NtupleMCReconstructibleTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleMCReconstructibleTool::eval - ERROR, no Particle found!\n");
    return;
  }

  StoreArray<MCParticle> mcParticles;

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_iReconstructible[iProduct] = 0;
    m_iSeenInPXD[iProduct] = 0;
    m_iSeenInSVD[iProduct] = 0;
    m_iSeenInCDC[iProduct] = 0;
    m_iSeenInTOP[iProduct] = 0;
    m_iLastSeenInECL[iProduct] = 0;
    m_iLastSeenInKLM[iProduct] = 0;

    const MCParticle* mcparticle = DataStore::getRelated<MCParticle>(selparticles[iProduct]);
    if (selparticles[iProduct]->getParticleType() == 6/*c_Composite*/) {
      B2INFO(boost::format("[NtupleMCReconstructibleTool]: this tool does not currently return the truth ID for composite particles : %d") % selparticles[iProduct]->getParticleType());
    } else if (!mcparticle) {
      printf("NtupleMCReconstructibleTool::eval - WARNING no truth match found for this reco particle!\n");
    } else {
      m_iReconstructible[iProduct] =  0;
      /* if it is a track make sure it went through the SVD for now */
      if (abs(mcparticle->getCharge()) > 0) {
        m_iReconstructible[iProduct] = (int) mcparticle->hasStatus(MCParticle::c_SeenInSVD);
        /* if it is a photon make sure it hit the ECL */
      } else if (abs(mcparticle->getPDG()) == 22) {
        m_iReconstructible[iProduct] = (int) mcparticle->hasStatus(MCParticle::c_LastSeenInECL);
        /* if it is a klong make sure it hit the ECL */
      } else if (abs(mcparticle->getPDG()) == 130) {
        m_iReconstructible[iProduct] = (int) mcparticle->hasStatus(MCParticle::c_LastSeenInECL);
      }

      m_iSeenInPXD[iProduct] = (int)mcparticle->hasStatus(MCParticle::c_SeenInPXD);
      m_iSeenInSVD[iProduct] = (int)mcparticle->hasStatus(MCParticle::c_SeenInSVD);
      m_iSeenInCDC[iProduct] = (int)mcparticle->hasStatus(MCParticle::c_SeenInCDC);
      m_iSeenInTOP[iProduct] = (int)mcparticle->hasStatus(MCParticle::c_SeenInTOP);
      m_iLastSeenInECL[iProduct] = (int)mcparticle->hasStatus(MCParticle::c_LastSeenInECL);
      m_iLastSeenInKLM[iProduct] = (int)mcparticle->hasStatus(MCParticle::c_LastSeenInKLM);


    }
  }
}
