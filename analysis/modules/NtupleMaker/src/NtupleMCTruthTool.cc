/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/modules/NtupleMaker/NtupleMCTruthTool.h>
#include <generators/dataobjects/MCParticle.h>
#include <TBranch.h>

void NtupleMCTruthTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();
  m_iTruthID = new int[nDecayProducts];
  m_iTruthIDMatch = new int[nDecayProducts];
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_TruthID").c_str(), &m_iTruthID[iProduct], (strNames[iProduct] + "_TruthID/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_TruthIDMatch").c_str(), &m_iTruthIDMatch[iProduct], (strNames[iProduct] + "_TruthIDMatch/I").c_str());
  }
}

void NtupleMCTruthTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleMCTruthTool::eval - ERROR, no Particle found!\n");
    return;
  }

  StoreArray<MCParticle> mcParticles();

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_iTruthID[iProduct] = 0;
    m_iTruthIDMatch[iProduct] = 0;

    const MCParticle* mcparticle = DataStore::getRelated<MCParticle>(selparticles[iProduct]);
    if (selparticles[iProduct]->getParticleType() == 6/*c_Composite*/) {
      B2INFO(boost::format("[NtupleMCTruthTool]: this tool does not currently return the truth ID for composite particles : %d") % selparticles[iProduct]->getParticleType());
    } else if (!mcparticle) {
      printf("NtupleMCTruthTool::eval - WARNING no truth match found for this reco particle!\n");
    } else {
      m_iTruthID[iProduct] =  mcparticle->getPDG();
      m_iTruthIDMatch[iProduct] = (mcparticle->getPDG() == selparticles[iProduct]->getPDGCode());
    }
  }
}
