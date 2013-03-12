/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/modules/NtupleMaker/NtupleDeltaEMbcTool.h>
#include <cmath>
#include <TBranch.h>
#include <TLorentzVector.h>
#include <TVector3.h>
#include <analysis/utility/physics.h>

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

  TVector3 v3_boost2cms = getCMSBoostVector();
  float fBeamEnergy = getCMSEnergy() / 2.0;

  TLorentzVector lv_b(selparticles[0]->get4Vector());
  lv_b.Boost(v3_boost2cms);

  m_fDeltaE = lv_b.E() - fBeamEnergy;
  m_fMbc    = sqrt((fBeamEnergy * fBeamEnergy) - lv_b.Vect().Mag2());
}




