/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Luigi Li Gioi                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleMCDeltaTTool.h>
#include <analysis/VariableManager/Variables.h>
#include <analysis/dataobjects/Vertex.h>
#include <cmath>
#include <TBranch.h>
#include <TLorentzVector.h>
#include <TVector3.h>

using namespace Belle2;
using namespace std;

void NtupleMCDeltaTTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;
  m_fMCTagPDG = 0;
  m_fTruthDeltaT = 0;

  m_tree->Branch((strNames[0] + "_mcTagPDG").c_str(), &m_fMCTagPDG, (strNames[0] + "_mcTagPDG/I").c_str());
  m_tree->Branch((strNames[0] + "_TruthDeltaT").c_str(), &m_fTruthDeltaT, (strNames[0] + "_TruthDeltaT/F").c_str());
}

void NtupleMCDeltaTTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleMCDeltaTTool::eval - no Particle found!");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  Vertex* Ver = selparticles[0]->getRelatedTo<Vertex>();
  if (Ver) {
    m_fMCTagPDG = Ver->getMCTagBFlavor();
    m_fTruthDeltaT = Ver->getMCDeltaT();
  }
}




