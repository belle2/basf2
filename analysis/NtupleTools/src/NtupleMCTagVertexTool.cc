/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Luigi Li Gioi                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleMCTagVertexTool.h>
#include <analysis/VariableManager/Variables.h>
#include <analysis/dataobjects/Vertex.h>
#include <cmath>
#include <TBranch.h>
#include <TLorentzVector.h>
#include <TVector3.h>

using namespace Belle2;
using namespace std;

void NtupleMCTagVertexTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;
  m_fMCTagVx = 0;
  m_fMCTagVy = 0;
  m_fMCTagVz = 0;

  m_tree->Branch((strNames[0] + "_TruthTagVx").c_str(), &m_fMCTagVx, (strNames[0] + "_TruthTagVx/F").c_str());
  m_tree->Branch((strNames[0] + "_TruthTagVy").c_str(), &m_fMCTagVy, (strNames[0] + "_TruthTagVy/F").c_str());
  m_tree->Branch((strNames[0] + "_TruthTagVz").c_str(), &m_fMCTagVz, (strNames[0] + "_TruthTagVz/F").c_str());
}

void NtupleMCTagVertexTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleMCTagVertexTool::eval - no Particle found!");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  Vertex* Ver = selparticles[0]->getRelatedTo<Vertex>();
  if (Ver) {
    m_fMCTagVx = Ver->getMCTagVertex().X();
    m_fMCTagVy = Ver->getMCTagVertex().Y();
    m_fMCTagVz = Ver->getMCTagVertex().Z();
  }
}




