/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Roca                                           *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleFlavorTagInfoTool.h>
#include <analysis/dataobjects/FlavorTagInfo.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleFlavorTagInfoTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;

  m_targetP = new float[8]();
  m_categoryP = new float[8]();
  m_isB = new float[8]();


  for (unsigned i = 0; i < sizeof(m_targetP); i++) {
    B2INFO("FILLING:");
    B2INFO(m_targetP[i]);
    B2INFO(m_categoryP[i]);
    B2INFO(m_categories[i]);
    m_tree->Branch((strNames[0] + "_" + m_categories[i] + "_targetP").c_str(), &m_targetP[i],
                   (strNames[0] + "_" + m_categories[i] + "_targetP/F").c_str());
    m_tree->Branch((strNames[0] + "_" + m_categories[i] + "_categoryP").c_str(), &m_categoryP[i],
                   (strNames[0] + "_" + m_categories[i] + "_categoryP/F").c_str());
    m_tree->Branch((strNames[0] + "_" + m_categories[i] + "_comesFromB").c_str(), &m_isB[i],
                   (strNames[0] + "_" + m_categories[i] + "_comesFromB/F").c_str());
  }
}

void NtupleFlavorTagInfoTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleFlavorTagInfoTool::eval - no Particle found!");
    return;
  }
  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  FlavorTagInfo* flavTag = selparticles[0]->getRelatedTo<FlavorTagInfo>();

  if (flavTag) {
    for (unsigned i = 0; i < flavTag->getCategories().size(); i++) {
      m_targetP[i] = flavTag->getTargProb()[i];
      m_categoryP[i] = flavTag->getCatProb()[i];
      m_isB[i] = float(flavTag->getIsB()[i]);
    }
  } else {
    B2ERROR("FlavorTagInfo Object does not exist");
  }
}




