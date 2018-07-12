/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Luigi Li Gioi                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleTagVertexTool.h>
#include <analysis/VariableManager/Variables.h>
#include <analysis/dataobjects/Vertex.h>
#include <cmath>
#include <TBranch.h>
#include <TLorentzVector.h>
#include <TVector3.h>

using namespace Belle2;
using namespace std;

void NtupleTagVertexTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;
  m_fTagVx = 0;
  m_fTagVy = 0;
  m_fTagVz = 0;
  m_fTagVex = 0;
  m_fTagVey = 0;
  m_fTagVez = 0;
  m_TagVPvalue = 0;
  m_TagVType = 0;
  m_TagVNTracks = -1;

  m_tree->Branch((strNames[0] + "_TagVx").c_str(), &m_fTagVx, (strNames[0] + "_TagVx/F").c_str());
  m_tree->Branch((strNames[0] + "_TagVy").c_str(), &m_fTagVy, (strNames[0] + "_TagVy/F").c_str());
  m_tree->Branch((strNames[0] + "_TagVz").c_str(), &m_fTagVz, (strNames[0] + "_TagVz/F").c_str());
  m_tree->Branch((strNames[0] + "_TagVex").c_str(), &m_fTagVex, (strNames[0] + "_TagVex/F").c_str());
  m_tree->Branch((strNames[0] + "_TagVey").c_str(), &m_fTagVey, (strNames[0] + "_TagVey/F").c_str());
  m_tree->Branch((strNames[0] + "_TagVez").c_str(), &m_fTagVez, (strNames[0] + "_TagVez/F").c_str());
  m_tree->Branch((strNames[0] + "_TagVPvalue").c_str(), &m_TagVPvalue, (strNames[0] + "_TagVPvalue/F").c_str());
  m_tree->Branch((strNames[0] + "_TagVType").c_str(), &m_TagVType, (strNames[0] + "_TagVType/I").c_str());
  m_tree->Branch((strNames[0] + "_TagVNTracks").c_str(), &m_TagVNTracks, (strNames[0] + "_TagVNTracks/I").c_str());
}

void NtupleTagVertexTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleTagVertexTool::eval - no Particle found!");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  Vertex* Ver = selparticles[0]->getRelatedTo<Vertex>();

  if (Ver) {
    m_fTagVx = Ver->getTagVertex().X();
    m_fTagVy = Ver->getTagVertex().Y();
    m_fTagVz = Ver->getTagVertex().Z();
    m_fTagVex = TMath::Sqrt(Ver->getTagVertexErrMatrix()[0][0]);
    m_fTagVey = TMath::Sqrt(Ver->getTagVertexErrMatrix()[1][1]);
    m_fTagVez = TMath::Sqrt(Ver->getTagVertexErrMatrix()[2][2]);
    m_TagVPvalue = Ver->getTagVertexPval();
    m_TagVType = Ver->getFitType();
    m_TagVNTracks = Ver->getNTracks();
  }
}




