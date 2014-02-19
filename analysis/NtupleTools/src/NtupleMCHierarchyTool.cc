/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleMCHierarchyTool.h>
#include <mdst/dataobjects/MCParticle.h>
#include <TBranch.h>

void NtupleMCHierarchyTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();
  m_iMotherID = new int[nDecayProducts];
  m_iGDMotherID = new int[nDecayProducts];
  m_iGDGDMotherID = new int[nDecayProducts];
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_MC_MOTHER_ID").c_str(), &m_iMotherID[iProduct], (strNames[iProduct] + "_MC_MOTHER_ID/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_MC_GD_MOTHER_ID").c_str(), &m_iGDMotherID[iProduct], (strNames[iProduct] + "_MC_GD_MOTHER_ID/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_MC_GD_GD_MOTHER_ID").c_str(), &m_iGDGDMotherID[iProduct], (strNames[iProduct] + "_MC_GD_GD_MOTHER_ID/I").c_str());
  }
}

void NtupleMCHierarchyTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleMCHierarchyTool::eval - ERROR, no Particle found!\n");
    return;
  }

  StoreArray<MCParticle> mcParticles;

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {

    const MCParticle* mcparticle = DataStore::getRelated<MCParticle>(selparticles[iProduct]);

    if (!mcparticle) {
      m_iMotherID[iProduct] = 0;
      m_iGDMotherID[iProduct] = 0;
      m_iGDGDMotherID[iProduct] = 0;
    } else {
      const MCParticle* mcparticleMother = mcparticle->getMother();

      if (!mcparticleMother) {
        m_iMotherID[iProduct] = 0;
        m_iGDMotherID[iProduct] = 0;
        m_iGDGDMotherID[iProduct] = 0;
      } else {
        m_iMotherID[iProduct] = mcparticleMother->getPDG();

        const MCParticle* mcparticleGDMother = mcparticleMother->getMother();
        if (!mcparticleGDMother) {
          m_iGDMotherID[iProduct] = 0;
          m_iGDGDMotherID[iProduct] = 0;

        } else {
          m_iGDMotherID[iProduct] = mcparticleGDMother->getPDG();

          const MCParticle* mcparticleGDGDMother = mcparticleGDMother->getMother();
          if (!mcparticleGDGDMother) {
            m_iGDGDMotherID[iProduct] = 0;

          } else {
            m_iGDGDMotherID[iProduct] = mcparticleGDGDMother->getPDG();
          }

        }


      }

    }
  }
}
