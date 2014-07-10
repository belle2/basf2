/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleDeltaEMbcTool.h>
#include <analysis/VariableManager/Variables.h>
#include <cmath>
#include <TBranch.h>
#include <TLorentzVector.h>
#include <TVector3.h>

using namespace Belle2;
using namespace std;

void NtupleDeltaEMbcTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;
  m_fDeltaE = 0;
  m_fMbc = 0;
  m_tree->Branch((strNames[0] + "_deltae").c_str(), &m_fDeltaE, (strNames[0] + "_deltae/F").c_str());
  m_tree->Branch((strNames[0] + "_mbc").c_str(), &m_fMbc, (strNames[0] + "_mbc/F").c_str());
}

void NtupleDeltaEMbcTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleDeltaEMbcTool::eval - ERROR, no Particle found!\n");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  m_fDeltaE = Variable::particleDeltaE(selparticles[0]);
  m_fMbc    = Variable::particleMbc(selparticles[0]);

}




