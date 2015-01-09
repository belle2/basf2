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
#include <mdst/dataobjects/MCParticle.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleMCReconstructibleTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();
  m_iReconstructible = new int[nDecayProducts];
  m_iSeenInPXD = new int[nDecayProducts];
  m_iSeenInSVD = new int[nDecayProducts];
  m_iSeenInCDC = new int[nDecayProducts];
  m_iSeenInTOP = new int[nDecayProducts];
  m_iSeenInARICH = new int[nDecayProducts];
  m_iSeenInECL = new int[nDecayProducts];
  m_iSeenInKLM = new int[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_Reconstructible").c_str(), &m_iReconstructible[iProduct], (strNames[iProduct] + "_Reconstructible/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_SeenInPXD").c_str(), &m_iSeenInPXD[iProduct], (strNames[iProduct] + "_SeenInPXD/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_SeenInSVD").c_str(), &m_iSeenInSVD[iProduct], (strNames[iProduct] + "_SeenInSVD/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_SeenInCDC").c_str(), &m_iSeenInCDC[iProduct], (strNames[iProduct] + "_SeenInCDC/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_SeenInTOP").c_str(), &m_iSeenInTOP[iProduct], (strNames[iProduct] + "_SeenInTOP/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_SeenInARICH").c_str(), &m_iSeenInARICH[iProduct], (strNames[iProduct] + "_SeenInARICH/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_SeenInECL").c_str(), &m_iSeenInECL[iProduct], (strNames[iProduct] + "_SeenInECL/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_SeenInKLM").c_str(), &m_iSeenInKLM[iProduct], (strNames[iProduct] + "_SeenInKLM/I").c_str());
  }
}

void NtupleMCReconstructibleTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleMCReconstructibleTool::eval - no Particle found!\n");
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
    m_iSeenInARICH[iProduct] = 0;
    m_iSeenInECL[iProduct] = 0;
    m_iSeenInKLM[iProduct] = 0;

    const MCParticle* mcparticle = selparticles[iProduct]->getRelated<MCParticle>();
    if (selparticles[iProduct]->getParticleType() == 6/*c_Composite*/) {
      B2INFO("[NtupleMCReconstructibleTool]: this tool does not currently return the truth ID for composite particles : " << selparticles[iProduct]->getParticleType());
    } else if (!mcparticle) {
      B2WARNING("NtupleMCReconstructibleTool::eval - no truth match found for this reco particle!\n");
    } else {
      m_iReconstructible[iProduct] =  0;
      /* if it is a track make sure it went through the SVD for now */
      if (abs(mcparticle->getCharge()) > 0) {
        m_iReconstructible[iProduct] = (int) mcparticle->hasSeenInDetector(Const::SVD);
        /* if it is a photon make sure it hit the ECL */
      } else if (abs(mcparticle->getPDG()) == 22) {
        m_iReconstructible[iProduct] = (int) mcparticle->hasSeenInDetector(Const::ECL);
        /* if it is a klong make sure it hit the ECL */
      } else if (abs(mcparticle->getPDG()) == 130) {
        m_iReconstructible[iProduct] = (int) mcparticle->hasSeenInDetector(Const::ECL);
      }

      m_iSeenInPXD[iProduct] = (int)mcparticle->hasSeenInDetector(Const::PXD);
      m_iSeenInSVD[iProduct] = (int)mcparticle->hasSeenInDetector(Const::SVD);
      m_iSeenInCDC[iProduct] = (int)mcparticle->hasSeenInDetector(Const::CDC);
      m_iSeenInTOP[iProduct] = (int)mcparticle->hasSeenInDetector(Const::TOP);
      m_iSeenInARICH[iProduct] = (int)mcparticle->hasSeenInDetector(Const::ARICH);
      m_iSeenInECL[iProduct] = (int)mcparticle->hasSeenInDetector(Const::ECL);
      m_iSeenInKLM[iProduct] = (int)mcparticle->hasSeenInDetector(Const::KLM);


    }
  }
}
