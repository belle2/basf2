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
#include <string>

using namespace Belle2;
using namespace std;

void NtupleMCHierarchyTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();
  vector<string> newstrNames;

  if (! m_strOption.empty()) {
    B2INFO("Option is: " << m_strOption);
    if (m_strOption == "Intermediate")
      m_InterMediate = true;
    else
      B2WARNING("INVALID option used, only allows * Intermediate *" << m_strOption);
  }

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    if (m_InterMediate) {
      newstrNames.push_back((strNames[iProduct] + "_Daughter0").c_str());
      newstrNames.push_back((strNames[iProduct] + "_Daughter1").c_str());
    } else newstrNames.push_back(strNames[iProduct]);
  }


  int newnDecayProducts = newstrNames.size();
  m_iMotherID = new int[newnDecayProducts];
  m_iGDMotherID = new int[newnDecayProducts];
  m_iGDGDMotherID = new int[newnDecayProducts];

  for (int iProduct = 0; iProduct < newnDecayProducts; iProduct++) {
    m_tree->Branch((newstrNames[iProduct] + "_MC_MOTHER_ID").c_str(),
                   &m_iMotherID[iProduct],
                   (newstrNames[iProduct] + "_MC_MOTHER_ID/I").c_str());
    m_tree->Branch((newstrNames[iProduct] + "_MC_GD_MOTHER_ID").c_str(),
                   &m_iGDMotherID[iProduct],
                   (newstrNames[iProduct] + "_MC_GD_MOTHER_ID/I").c_str());
    m_tree->Branch((newstrNames[iProduct] + "_MC_GD_GD_MOTHER_ID").c_str(),
                   &m_iGDGDMotherID[iProduct],
                   (newstrNames[iProduct] + "_MC_GD_GD_MOTHER_ID/I").c_str());
  }
}


void NtupleMCHierarchyTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleMCHierarchyTool::eval - no Particle found!");
    return;
  }

  StoreArray<MCParticle> mcParticles;
  vector<const Particle*> selparticles =
    m_decaydescriptor.getSelectionParticles(particle);
  vector<const Particle*> newselparticles;

  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    if (!m_InterMediate) { newselparticles.push_back(selparticles[iProduct]); }
    else {
      const std::vector<Particle*> daughters = selparticles[iProduct]->getDaughters();
      int nDaughters = daughters.size();
      for (int iDaughter = 0; iDaughter < nDaughters; ++iDaughter) {
        if (nDaughters == 2) {newselparticles.push_back(daughters[iDaughter]);}
        else {
          B2ERROR("NtupleMCHierarchyTool::eval - More than two daughters!");
          return;
        }
      }
    }
  }


  int newnDecayProducts = newselparticles.size();
  for (int iProduct = 0; iProduct < newnDecayProducts; iProduct++) {
    const MCParticle* mcparticle = newselparticles[iProduct]->getRelated<MCParticle>();
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

