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
#include <analysis/dataobjects/FlavorTaggerInfo.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

/* WORK IN PROGRESS */

void NtupleFlavorTagInfoTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;

  m_targetP = new float[8]();
  m_categoryP = new float[8]();
  m_isFromB = new float[8]();
  m_D0 = new float[8]();
  m_Z0 = new float[8]();
  m_P = new float[8]();
  m_prodPointResolutionZ = new float[8]();

  m_goodTracksPurityFT = 0;
  m_goodTracksPurityROE = 0;
  m_badTracksROE = 0;
  m_goodTracksROE = 0;
  m_badTracksFT = 0;
  m_goodTracksFT = 0;

  for (unsigned i = 0; i < sizeof(m_targetP); i++) {
    m_tree->Branch((strNames[0] + "_" + m_categories[i] + "_targetP").c_str(), &m_targetP[i],
                   (strNames[0] + "_" + m_categories[i] + "_targetP/F").c_str());
    m_tree->Branch((strNames[0] + "_" + m_categories[i] + "_categoryP").c_str(), &m_categoryP[i],
                   (strNames[0] + "_" + m_categories[i] + "_categoryP/F").c_str());
    m_tree->Branch((strNames[0] + "_" + m_categories[i] + "_comesFromB").c_str(), &m_isFromB[i],
                   (strNames[0] + "_" + m_categories[i] + "_comesFromB/F").c_str());
    m_tree->Branch((strNames[0] + "_" + m_categories[i] + "_P").c_str(), &m_P[i],
                   (strNames[0] + "_" + m_categories[i] + "_P/F").c_str());
    m_tree->Branch((strNames[0] + "_" + m_categories[i] + "_D0").c_str(), &m_D0[i],
                   (strNames[0] + "_" + m_categories[i] + "_D0/F").c_str());
    m_tree->Branch((strNames[0] + "_" + m_categories[i] + "_Z0").c_str(), &m_Z0[i],
                   (strNames[0] + "_" + m_categories[i] + "_Z0/F").c_str());
    m_tree->Branch((strNames[0] + "_" + m_categories[i] + "m_prodPointResolutionZ").c_str(), &m_prodPointResolutionZ[i],
                   (strNames[0] + "_" + m_categories[i] + "m_prodPointResolutionZ/F").c_str());
  }

  m_tree->Branch((strNames[0] + "_goodTracksPurityFT").c_str(), &m_goodTracksPurityFT,
                 (strNames[0] + "_goodTracksPurityFT/F").c_str());
  m_tree->Branch((strNames[0] + "_goodTracksPurityROE").c_str(), &m_goodTracksPurityROE,
                 (strNames[0] + "_goodTracksPurityROE/F").c_str());


  m_tree->Branch((strNames[0] + "_goodTracksROE").c_str(), &m_goodTracksROE,
                 (strNames[0] + "_goodTracksROE/F").c_str());
  m_tree->Branch((strNames[0] + "_badTracksROE").c_str(), &m_badTracksROE,
                 (strNames[0] + "_badTracksROE/F").c_str());
  m_tree->Branch((strNames[0] + "_goodTracksFT").c_str(), &m_goodTracksFT,
                 (strNames[0] + "_goodTracksFT/F").c_str());
  m_tree->Branch((strNames[0] + "_badTracksFT").c_str(), &m_badTracksFT,
                 (strNames[0] + "_badTracksFT/F").c_str());
}

void NtupleFlavorTagInfoTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleFlavorTagInfoTool::eval - no Particle found!");
    return;
  }
  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  auto* flavTag = selparticles[0]->getRelatedTo<FlavorTaggerInfo>();

  if (flavTag) {
    for (unsigned i = 0; i < flavTag->getCategories().size(); i++) {
      m_targetP[i] = flavTag->getTargProb()[i];
      m_categoryP[i] = flavTag->getCatProb()[i];
      m_isFromB[i] = flavTag->getIsFromB()[i];
      m_P[i] = flavTag->getP()[i];
      m_Z0[i] = flavTag->getZ0()[i];
      m_D0[i] = flavTag->getD0()[i];
      m_prodPointResolutionZ[i] = flavTag->getProdPointResolutionZ()[i];
    }
    m_goodTracksPurityFT = flavTag->getGoodTracksPurityFT();
    m_goodTracksPurityROE = flavTag->getGoodTracksPurityROE();
    m_badTracksROE = float(flavTag->getBadTracksROE());
    m_goodTracksROE = float(flavTag->getGoodTracksROE());
    m_badTracksFT = float(flavTag->getBadTracksFT());
    m_goodTracksFT = float(flavTag->getGoodTracksFT());

  } else {
    B2ERROR("FlavorTaggerInfo Object does not exist");
  }
}




