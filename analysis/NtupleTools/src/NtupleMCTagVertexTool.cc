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
#include <analysis/utility/PSelectorFunctions.h>
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
  m_fMCTagPDG = 0;

  m_tree->Branch((strNames[0] + "_TruthTagVx").c_str(), &m_fMCTagVx, (strNames[0] + "_TruthTagVx/F").c_str());
  m_tree->Branch((strNames[0] + "_TruthTagVy").c_str(), &m_fMCTagVy, (strNames[0] + "_TruthTagVy/F").c_str());
  m_tree->Branch((strNames[0] + "_TruthTagVz").c_str(), &m_fMCTagVz, (strNames[0] + "_TruthTagVz/F").c_str());
  m_tree->Branch((strNames[0] + "_mcTagPDG").c_str(), &m_fMCTagPDG, (strNames[0] + "_mcTagPDG/I").c_str());

}

void NtupleMCTagVertexTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleMCTagVertexTool::eval - ERROR, no Particle found!\n");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  m_fMCTagVx = selparticles[0]->getExtraInfo("MCTagVx");
  m_fMCTagVy = selparticles[0]->getExtraInfo("MCTagVy");
  m_fMCTagVz = selparticles[0]->getExtraInfo("MCTagVz");
  m_fMCTagPDG = selparticles[0]->getExtraInfo("MCTagPDG");

}




