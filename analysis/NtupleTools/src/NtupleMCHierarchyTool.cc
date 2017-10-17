/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo                        *
*               Vishal Bhardwaj                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleMCHierarchyTool.h>
#include <mdst/dataobjects/MCParticle.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleMCHierarchyTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();

  if (! m_strOption.empty()) {
    B2INFO("Option is: " << m_strOption);
    if (m_strOption == "InterMediate")
      m_InterMediate = true;
    else
      B2WARNING("Invalid option used, only allows * InterMediate *" << m_strOption);
  }



  m_iMotherID = new int[nDecayProducts];
  m_iGDMotherID = new int[nDecayProducts];
  m_iGDGDMotherID = new int[nDecayProducts];

  if (m_InterMediate) {
    m_iMotherID1 = new int[nDecayProducts];
    m_iGDMotherID1 = new int[nDecayProducts];
    m_iGDGDMotherID1 = new int[nDecayProducts];
    m_iMCINFO = new int[nDecayProducts];
  }

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {

    if (m_InterMediate) {
      m_tree->Branch((strNames[iProduct] + "_dau0_MC_MOTHER_ID").c_str(), &m_iMotherID[iProduct],
                     (strNames[iProduct] + "_dau0_MC_MOTHER_ID/I").c_str());
      m_tree->Branch((strNames[iProduct] + "_dau0_MC_GD_MOTHER_ID").c_str(), &m_iGDMotherID[iProduct],
                     (strNames[iProduct] + "_dau0_MC_GD_MOTHER_ID/I").c_str());
      m_tree->Branch((strNames[iProduct] + "_dau0_MC_GD_GD_MOTHER_ID").c_str(), &m_iGDGDMotherID[iProduct],
                     (strNames[iProduct] + "_dau0_MC_GD_GD_MOTHER_ID/I").c_str());
      m_tree->Branch((strNames[iProduct] + "_dau1_MC_MOTHER_ID").c_str(), &m_iMotherID1[iProduct],
                     (strNames[iProduct] + "_dau1_MC_MOTHER_ID/I").c_str());
      m_tree->Branch((strNames[iProduct] + "_dau1_MC_GD_MOTHER_ID").c_str(), &m_iGDMotherID1[iProduct],
                     (strNames[iProduct] + "_dau1_MC_GD_MOTHER_ID/I").c_str());
      m_tree->Branch((strNames[iProduct] + "_dau1_MC_GD_GD_MOTHER_ID").c_str(), &m_iGDGDMotherID1[iProduct],
                     (strNames[iProduct] + "_dau1_MC_GD_GD_MOTHER_ID/I").c_str());
      m_tree->Branch((strNames[iProduct] + "_MC_daughtersINFO").c_str(), &m_iMCINFO[iProduct],
                     (strNames[iProduct] + "_MC_daughtersINFO/I").c_str());

    } else {
      m_tree->Branch((strNames[iProduct] + "_MC_MOTHER_ID").c_str(), &m_iMotherID[iProduct],
                     (strNames[iProduct] + "_MC_MOTHER_ID/I").c_str());
      m_tree->Branch((strNames[iProduct] + "_MC_GD_MOTHER_ID").c_str(), &m_iGDMotherID[iProduct],
                     (strNames[iProduct] + "_MC_GD_MOTHER_ID/I").c_str());
      m_tree->Branch((strNames[iProduct] + "_MC_GD_GD_MOTHER_ID").c_str(), &m_iGDGDMotherID[iProduct],
                     (strNames[iProduct] + "_MC_GD_GD_MOTHER_ID/I").c_str());

    }



  }
}

void NtupleMCHierarchyTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleMCHierarchyTool::eval - no Particle found!");
    return;
  }

  StoreArray<MCParticle> mcParticles;

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {

    if (m_InterMediate) {
      int ParticlePDGcode = selparticles[iProduct]->getPDGCode();
      const std::vector<Particle*> daughters = selparticles[iProduct]->getDaughters();
      if (daughters.size() != 2) {
        m_iMotherID[iProduct] = 0;
        m_iGDMotherID[iProduct] = 0;
        m_iGDGDMotherID[iProduct] = 0;
      }
      const MCParticle* mcparticle = daughters[0]->getRelated<MCParticle>();
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

      const MCParticle* mcparticle2 = daughters[1]->getRelated<MCParticle>();
      if (!mcparticle2) {
        m_iMotherID1[iProduct] = 0;
        m_iGDMotherID1[iProduct] = 0;
        m_iGDGDMotherID1[iProduct] = 0;
      } else {
        const MCParticle* mcparticleMother2 = mcparticle2->getMother();

        if (!mcparticleMother2) {
          m_iMotherID1[iProduct] = 0;
          m_iGDMotherID1[iProduct] = 0;
          m_iGDGDMotherID1[iProduct] = 0;
        } else {
          m_iMotherID1[iProduct] = mcparticleMother2->getPDG();

          const MCParticle* mcparticleGDMother2 = mcparticleMother2->getMother();
          if (!mcparticleGDMother2) {
            m_iGDMotherID1[iProduct] = 0;
            m_iGDGDMotherID1[iProduct] = 0;

          } else {
            m_iGDMotherID1[iProduct] = mcparticleGDMother2->getPDG();

            const MCParticle* mcparticleGDGDMother2 = mcparticleGDMother2->getMother();
            if (!mcparticleGDGDMother2) {
              m_iGDGDMotherID1[iProduct] = 0;

            } else {
              m_iGDGDMotherID1[iProduct] = mcparticleGDGDMother2->getPDG();
            }
          }
        }


        m_iMCINFO[iProduct] = 0;
        if (m_iMotherID[iProduct] == ParticlePDGcode)
          m_iMCINFO[iProduct] = 1;
        // First daughter is from the expected particle

        if (m_iMotherID1[iProduct] == ParticlePDGcode)
          m_iMCINFO[iProduct] = 10;
        // Second daughter is from the expected reconstructed particle

        if (m_iMotherID[iProduct] != 0 && m_iMotherID[iProduct] != 0) {
          if (m_iMotherID[iProduct] == m_iMotherID1[iProduct] &&
              m_iMotherID[iProduct] == ParticlePDGcode) {
            if (m_iGDMotherID[iProduct] ==
                m_iGDMotherID1[iProduct]) {
              if (m_iGDGDMotherID[iProduct] ==
                  m_iGDGDMotherID1[iProduct]) {
                m_iMCINFO[iProduct] = 33;
                // Both daughters same Mother, GM, GGM
              } else {
                m_iMCINFO[iProduct] = 22;
                // Both daughters same Mothers and GMs but diff GGMs ?
              }
            } else {
              m_iMCINFO[iProduct] = 11;
              // Both daughters same Mothers but diff GMs
            }
          }
        }
      }
    } else {

      const MCParticle* mcparticle = selparticles[iProduct]->getRelated<MCParticle>();


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
}

