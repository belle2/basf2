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
  m_isFromB = new float[8]();
  m_D0 = new float[8]();
  m_Z0 = new float[8]();
  m_P = new float[8]();
  m_MCvertex = new float[8]();

  m_isFromBGeneral = 0;
  m_ROEComesFromB = 0;
  m_goodTracksPurityFT = 0;
  m_goodBadTracksRatioFT = 0;
  m_FTandROEGoodTracksRatio = 0;
  m_goodTracksPurityROE = 0;
  m_goodBadTracksRatioROE = 0;
  m_badTracksROE = 0;
  m_goodTracksROE = 0;
  m_badTracksFT = 0;
  m_goodTracksFT = 0;

  for (unsigned i = 0; i < sizeof(m_targetP); i++) {
    +
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
    m_tree->Branch((strNames[0] + "_" + m_categories[i] + "_MCvertex").c_str(), &m_MCvertex[i],
                   (strNames[0] + "_" + m_categories[i] + "_MCvertex/F").c_str());
  }
  m_tree->Branch((strNames[0] + "_comesFromBGeneral").c_str(), &m_isFromBGeneral,
                 (strNames[0] + "_comesFromBGeneral/F").c_str());
  m_tree->Branch((strNames[0] + "_ROEComesFromB").c_str(), &m_ROEComesFromB,
                 (strNames[0] + "_ROEComesFromB/F").c_str());
  m_tree->Branch((strNames[0] + "_goodTracksPurityFT").c_str(), &m_goodTracksPurityFT,
                 (strNames[0] + "_goodTracksPurityFT/F").c_str());
  m_tree->Branch((strNames[0] + "_goodBadTracksRatioFT").c_str(), &m_goodBadTracksRatioFT,
                 (strNames[0] + "_goodBadTracksRatioFT").c_str());
  m_tree->Branch((strNames[0] + "_goodTracksPurityROE").c_str(), &m_goodTracksPurityROE,
                 (strNames[0] + "_goodTracksPurityROE/F").c_str());
  m_tree->Branch((strNames[0] + "_goodBadTracksRatioROE").c_str(), &m_goodBadTracksRatioROE,
                 (strNames[0] + "_goodBadTracksRatioROE/F").c_str());
  m_tree->Branch((strNames[0] + "_FTandROEGoodTracksRatio").c_str(), &m_FTandROEGoodTracksRatio,
                 (strNames[0] + "_FTandROEGoodTracksRatio/F").c_str());

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

  FlavorTagInfo* flavTag = selparticles[0]->getRelatedTo<FlavorTagInfo>();

  if (flavTag) {
    for (unsigned i = 0; i < flavTag->getCategories().size(); i++) {
      m_targetP[i] = flavTag->getTargProb()[i];
      m_categoryP[i] = flavTag->getCatProb()[i];
      m_isFromB[i] = flavTag->getIsFromB()[i];
      m_P[i] = flavTag->getP()[i];
      m_Z0[i] = flavTag->getZ0()[i];
      m_D0[i] = flavTag->getD0()[i];
      m_MCvertex[i] = flavTag->getProdVertexMotherZ()[i];
    }
    m_isFromBGeneral = float(flavTag->getIsFromBGeneral());
    m_ROEComesFromB = float(flavTag->getROEComesFromB());
    m_goodTracksPurityFT = flavTag->getGoodTracksPurityFT();
    m_goodBadTracksRatioFT = flavTag->getGoodBadTracksRatioFT();
    m_goodTracksPurityROE = flavTag->getGoodTracksPurityROE();
    m_goodBadTracksRatioROE = flavTag->getGoodBadTracksRatioROE();
    m_FTandROEGoodTracksRatio = flavTag->getFTandROEGoodTracksRatio();
    m_badTracksROE = float(flavTag->getBadTracksROE());
    m_goodTracksROE = float(flavTag->getGoodTracksROE());
    m_badTracksFT = float(flavTag->getBadTracksFT());
    m_goodTracksFT = float(flavTag->getGoodTracksFT());

  } else {
    B2ERROR("FlavorTagInfo Object does not exist");
  }
}




