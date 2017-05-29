/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Luigi Li Gioi                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleDeltaTTool.h>
#include <analysis/VariableManager/Variables.h>
#include <analysis/dataobjects/Vertex.h>
#include <cmath>
#include <TBranch.h>
#include <TLorentzVector.h>
#include <TVector3.h>

using namespace Belle2;
using namespace std;

void NtupleDeltaTTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;
  m_fDeltaT = 0;
  m_fDeltaTErr = 0;

  m_tree->Branch((strNames[0] + "_DeltaT").c_str(), &m_fDeltaT, (strNames[0] + "_DeltaT/F").c_str());
  m_tree->Branch((strNames[0] + "_DeltaTErr").c_str(), &m_fDeltaTErr, (strNames[0] + "_DeltaTErr/F").c_str());
}

void NtupleDeltaTTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleDeltaTTool::eval - no Particle found!");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  Vertex* Ver = selparticles[0]->getRelatedTo<Vertex>();

  if (Ver) {
    m_fDeltaT = Ver->getDeltaT();
    m_fDeltaTErr = Ver->getDeltaTErr();
  }
}




